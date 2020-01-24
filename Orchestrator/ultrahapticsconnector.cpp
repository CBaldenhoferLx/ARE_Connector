#include "ultrahapticsconnector.h"

#include <QDebug>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

UltrahapticsConnector::UltrahapticsConnector(QObject *parent) : DataReceiver(parent)
{
    qDebug() << Q_FUNC_INFO;
}

void UltrahapticsConnector::start() {
    qDebug() << Q_FUNC_INFO;

    m_future = QtConcurrent::run(this, &UltrahapticsConnector::runLoop);
}

void UltrahapticsConnector::stop() {
    qDebug() << Q_FUNC_INFO;

    if (m_future.isRunning()) m_future.cancel();
}

void UltrahapticsConnector::sendData(SerialProtocol::SerialProtocolAction action) {
    qDebug() << Q_FUNC_INFO << action.action;

    switch(action.action) {
    case SerialProtocol::ACTION_TOUCH_TRIGGERED:
        double v = qBound(BUTTON_STRENGTH_OFF, action.param, BUTTON_STRENGTH_FULL);
        m_buttonStrength = v;
        break;
    }
}

void UltrahapticsConnector::setButtonStrength(quint8 strength) {
    qDebug() << Q_FUNC_INFO << strength;

    m_buttonStrength = strength;
}

// Call this method externally to enable/disable the hand scan sensation.
// This swaps the Ultrahaptics emission callback, to output either a hand scan or a circle onto the hand.
void UltrahapticsConnector::setScanEnabled(bool enabled) {
    qDebug() << Q_FUNC_INFO << enabled;
    m_scanEnabled = enabled;

    m_emitter->stop();
    if (enabled)
    {
        m_emitter->setEmissionCallback(hand_scan_emitter_callback, &m_hand_scan_data);
    }
    else
    {
        // When we turn off hand scan sensation, switch back to the circle sensation
        m_emitter->setEmissionCallback(circle_emitter_callback, &m_circle_data);
    }
    m_emitter->start();
}

// Callback function for filling out complete device output states through time
void UltrahapticsConnector::circle_emitter_callback(const Ultrahaptics::TimePointStreaming::Emitter &timepoint_emitter,
                         Ultrahaptics::TimePointStreaming::OutputInterval &interval,
                         const Ultrahaptics::HostTimePoint &submission_deadline,
                         void *user_pointer)
{
    // Cast the user pointer to the struct that describes the control point behaviour
    Circle *circle = static_cast<Circle*>(user_pointer);

    // Loop through the samples in this interval
    for (auto& sample : interval)
    {
        const Seconds t = sample - start_time;
        const Ultrahaptics::Vector3 position = circle->evaluateAt(t);

        // Set the position and intensity of the persistent control point to that of the modulated wave at this point in time.
        sample.persistentControlPoint(0).setPosition(circle->offset + position);
        sample.persistentControlPoint(0).setIntensity(circle->intensity);

        if (circle->enable) {
            sample.persistentControlPoint(0).enable();
        } else {
            sample.persistentControlPoint(0).disable();
        }
    }
}

void UltrahapticsConnector::hand_scan_emitter_callback(const Ultrahaptics::TimePointStreaming::Emitter &timepoint_emitter,
                         Ultrahaptics::TimePointStreaming::OutputInterval &interval,
                         const Ultrahaptics::HostTimePoint &submission_deadline,
                         void *user_pointer)
{
    // Cast the user pointer to the struct that describes the behaviour
    struct HandScan *hand_scan = (struct HandScan *)user_pointer;

    // Set interval offset between control point samples
    const float interval_offset_x = hand_scan->forcefield_width * (hand_scan->forcefield_frequency_x / ((double)timepoint_emitter.getSampleRate()));
    const float interval_offset_y = hand_scan->forcefield_height * (hand_scan->forcefield_frequency_y / ((double)timepoint_emitter.getSampleRate()));

    // Initialise the control point sample before the first to be at the start of the very first interval
    if (hand_scan->is_initial_interval)
    {
        // Ensure the next sample offset is zero
        hand_scan->previous_sample_offset_x = -interval_offset_x;
        hand_scan->previous_sample_offset_y = -interval_offset_y;
        // Mark any later interval as not initial
        hand_scan->is_initial_interval = false;
    }

    // Loop through time, setting control points
    for (Ultrahaptics::TimePointStreaming::OutputInterval::iterator
       it = interval.begin(); it < interval.end(); ++it)
    {
        if (!hand_scan->hand.isHandPresent())
        {
            it->persistentControlPoint(0).disable();
            continue;
        }
        else
        {
            it->persistentControlPoint(0).enable();
        }

        // Set up the next distance to offset the point along its axis by
        float next_sample_offset_x = hand_scan->previous_sample_offset_x + interval_offset_x * (hand_scan->direction_x ? 1 : -1);
        float next_sample_offset_y = hand_scan->previous_sample_offset_y + interval_offset_y * (hand_scan->direction_y ? 1 : -1);

        Ultrahaptics::Vector3 palmXAxisInDeviceSpace = hand_scan->hand.getXAxis() * 5.f * Ultrahaptics::Units::cm;

        // Work out the point position along the x-axis of the palm
        float dx = next_sample_offset_x - hand_scan->hand.getPalmPosition().x;
        float distance_along_palm_x_vector = dx / palmXAxisInDeviceSpace.x;
        float z_component = distance_along_palm_x_vector * palmXAxisInDeviceSpace.z;

        // Point moves back and forth along the x-axis of the palm
        if (hand_scan->direction_x)
        {
            hand_scan->position.x = hand_scan->hand.getPalmPosition().x + next_sample_offset_x;
        }
        else
        {
            hand_scan->position.x = hand_scan->hand.getPalmPosition().x - next_sample_offset_x;
        }

        if (hand_scan->direction_y)
        {
            hand_scan->position.y = hand_scan->hand.getPalmPosition().y + next_sample_offset_y;
        }
        else
        {
            hand_scan->position.y = hand_scan->hand.getPalmPosition().y - next_sample_offset_y;
        }

        hand_scan->position.z = z_component + hand_scan->hand.getPalmPosition().z;

        it->persistentControlPoint(0).setPosition(hand_scan->position);
        it->persistentControlPoint(0).setIntensity(1.f);

        // Update previous sample offset for next loop iteration
        hand_scan->previous_sample_offset_x = next_sample_offset_x;
        hand_scan->previous_sample_offset_y = next_sample_offset_y;

        // Check if we reached the edge of the forcefield projection width, and reverse direction if so
        if( fabs(hand_scan->previous_sample_offset_x) > hand_scan->forcefield_width / 2 )
        {
            hand_scan->direction_x = !hand_scan->direction_x;
        }

        if( fabs(hand_scan->previous_sample_offset_y) > hand_scan->forcefield_height / 2 )
        {
            hand_scan->direction_y = !hand_scan->direction_y;
        }
    }
}

void UltrahapticsConnector::runLoop() {
    // Create a time point streaming emitter for one control point
    m_emitter = new Ultrahaptics::TimePointStreaming::Emitter();
    m_emitter->setMaximumControlPointCount(1);
    m_emitter->setSampleRate(10000); // reduce device sample rate for smoother performance on Windows USB stack

    // Load the appropriate Leap Motion alignment matrix for this kit
    Ultrahaptics::Alignment alignment = m_emitter->getDeviceInfo().getDefaultAlignment();

    Leap::Controller leap_controller;
    leap_controller.addListener(m_hand_scan_data.hand);

    // The default output will be to do a handscan on the hand
    m_emitter->setEmissionCallback(hand_scan_emitter_callback, &m_hand_scan_data);
    m_emitter->start();

    for (;;)
    {
        if (m_scanEnabled)
        {
            // User's first interaction with the demo. Perform a hand scan haptic effect when user's hand is in the correct place
            // We don't need to do anything else here - all calculations happen in hand scan emitter callback and LeapListening onFrame handler.
        }
        else
        {
            // Update the m_circle_data struct with the desired circle position data
            if (m_buttonStrength>0)
            {
                Leap::HandList hands = leap_controller.frame().hands();
                Leap::Finger indexFinger;

                // find the index finger
                if (!hands.isEmpty()) {
                    Leap::FingerList theFingers = hands.frontmost().fingers().extended();

                    qDebug() << "Fingers:" << theFingers.count() << "confidence" << hands.frontmost().confidence();

                    for(Leap::FingerList::const_iterator fl = theFingers.begin(); fl != theFingers.end(); fl++) {
                        const Leap::Finger finger = *fl;
                        if (finger.type()==Leap::Finger::Type::TYPE_INDEX) {
                            qDebug() << "Index finger found";
                            indexFinger = finger;
                            break;
                        }
                    }
                }

                if (indexFinger.isValid())
                {
                    Leap::Vector leap_palm_position = indexFinger.tipPosition();
                    Leap::Vector leap_palm_normal = indexFinger.hand().palmNormal();
                    Leap::Vector leap_palm_direction = indexFinger.hand().direction();

                    // Convert to Ultrahaptics vectors, normal is negated as leap normal points down.
                    Ultrahaptics::Vector3 uh_palm_position(leap_palm_position.x, leap_palm_position.y, leap_palm_position.z);
                    Ultrahaptics::Vector3 uh_palm_normal(-leap_palm_normal.x, -leap_palm_normal.y, -leap_palm_normal.z);
                    Ultrahaptics::Vector3 uh_palm_direction(leap_palm_direction.x, leap_palm_direction.y, leap_palm_direction.z);

                    // Convert to device space from leap space.
                    Ultrahaptics::Vector3 device_palm_position = alignment.fromTrackingPositionToDevicePosition(uh_palm_position);
                    Ultrahaptics::Vector3 device_palm_normal = alignment.fromTrackingDirectionToDeviceDirection(uh_palm_normal).normalize();
                    Ultrahaptics::Vector3 device_palm_direction = alignment.fromTrackingDirectionToDeviceDirection(uh_palm_direction).normalize();

                    // These can then be converted to be a unit axis on the palm of the hand.
                    Ultrahaptics::Vector3 device_palm_z = device_palm_normal;                             // Unit Z direction.
                    Ultrahaptics::Vector3 device_palm_y = device_palm_direction;                          // Unit Y direction.
                    Ultrahaptics::Vector3 device_palm_x = device_palm_y.cross(device_palm_z).normalize(); // Unit X direction.

                    Leap::Vector boneV = indexFinger.bone(Leap::Bone::Type::TYPE_DISTAL).center();
                    Ultrahaptics::Vector3 bone_pos(boneV.x, boneV.y, boneV.z);
                    Ultrahaptics::Vector3 position = alignment.fromTrackingPositionToDevicePosition(bone_pos);

                    float strengthFactor = (BUTTON_STRENGTH_BASE + m_buttonStrength) / 10.0f;
                    m_circle_data.intensity = 1.0f * strengthFactor;
                    m_circle_data.offset = position;
                    m_circle_data.enable = true;
                }
                else
                {
                    m_circle_data.enable = false;
                }
            }
            else
            {
                m_circle_data.enable = false;
            }
        }

        QThread::msleep(10);
    }
}
