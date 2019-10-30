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

void UltrahapticsConnector::sendData(Protocol::ProtocolAction action) {
    qDebug() << Q_FUNC_INFO << action.action;

    switch(action.action) {
    case Protocol::ACTION_TOUCH_TRIGGERED:
        double v = qBound(BUTTON_STRENGTH_OFF, action.param, BUTTON_STRENGTH_FULL);
        m_buttonStrength = v;
        break;
    }
}

void UltrahapticsConnector::setButtonStrength(quint8 strength) {
    qDebug() << Q_FUNC_INFO << strength;

    m_buttonStrength = strength;
}



// Callback function for filling out complete device output states through time
void UltrahapticsConnector::my_emitter_callback(const Ultrahaptics::TimePointStreaming::Emitter &timepoint_emitter,
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

void UltrahapticsConnector::runLoop() {
    // Create a time point streaming emitter
    Ultrahaptics::TimePointStreaming::Emitter emitter;

    Leap::Controller leap_controller;
    // Load the appropriate Leap Motion alignment matrix for this kit
    Ultrahaptics::Alignment alignment = emitter.getDeviceInfo().getDefaultAlignment();

    // Set frequency to 200 Hertz and maximum intensity
    float frequency = 200.0f * Ultrahaptics::Units::hertz;
    float intensity = 1.0f;

    // Set the maximum control point count
    emitter.setMaximumControlPointCount(1);

    // Create a structure containing our control point data and fill it in
    Circle circle;

    // Set control point 20cm above the centre of the array at the radius
    //circle.position = Ultrahaptics::Vector3(2.0f * Ultrahaptics::Units::centimetres, 0.0f, 20.0f * Ultrahaptics::Units::centimetres);
    circle.position = Ultrahaptics::Vector3(0, 0, 0);
    // Set the amplitude of the modulation of the wave to one (full modulation depth)
    circle.intensity = 1.0f;
    // Set the radius of the circle that the point is traversing
    circle.radius = 2.0f * Ultrahaptics::Units::centimetres;
    // Set how many times the point traverses the circle every second
    circle.frequency = 100.0f;

    // Set the callback function to the callback written above
    emitter.setEmissionCallback(my_emitter_callback, &circle);

    // Start the array
    emitter.start();

    for (;;)
    {
        if (m_buttonStrength>0) {
            // Get all the hand positions from the leap and position a focal point on each.

            Leap::HandList hands = leap_controller.frame().hands();
            Leap::Finger indexFinger;

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

                // Use these to create a point at 2cm x 2cm from the centre of the palm
                //Ultrahaptics::Vector3 position = device_palm_position + (2 * Ultrahaptics::Units::cm * device_palm_x) + (2 * Ultrahaptics::Units::cm * device_palm_y);
                //Ultrahaptics::Vector3 position = device_palm_position - (2 * Ultrahaptics::Units::cm * device_palm_x) - (2 * Ultrahaptics::Units::cm * device_palm_y);
                //Ultrahaptics::Vector3 position = device_palm_position;

                Leap::Vector boneV = indexFinger.bone(Leap::Bone::Type::TYPE_DISTAL).center();
                Ultrahaptics::Vector3 bone_pos(boneV.x, boneV.y, boneV.z);
                Ultrahaptics::Vector3 position = alignment.fromTrackingPositionToDevicePosition(bone_pos);

                bone_pos.z = bone_pos.z - (10 * Ultrahaptics::Units::mm);

                // Create the control point and emit.
                float strengthFactor = (BUTTON_STRENGTH_BASE + m_buttonStrength) / 10.0f;
                Ultrahaptics::ControlPoint point(position, intensity * strengthFactor, frequency);

                circle.intensity = intensity * strengthFactor;
                circle.offset = position;

                circle.enable = true;
            }
            else
            {
                circle.enable = false;
            }
        } else {
            circle.enable = false;
        }

        QThread::msleep(10);
    }
}

/*
void UltrahapticsConnector::runLoop() {
    // Create an emitter and a leap controller.
    Ultrahaptics::AmplitudeModulation::Emitter emitter;
    Leap::Controller leap_controller;
    // Load the appropriate Leap Motion alignment matrix for this kit
    Ultrahaptics::Alignment alignment = emitter.getDeviceInfo().getDefaultAlignment();

    // Set frequency to 200 Hertz and maximum intensity
    float frequency = 200.0f * Ultrahaptics::Units::hertz;
    float intensity = 1.0f;

    for (;;)
    {
        if (m_buttonStrength>0) {
            // Get all the hand positions from the leap and position a focal point on each.
            Leap::HandList hands = leap_controller.frame().hands();
            Leap::Finger indexFinger;

            if (!hands.isEmpty()) {
                Leap::FingerList theFingers = hands.frontmost().fingers().extended();

                qDebug() << "Fingers:" << theFingers.count();

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

                // Use these to create a point at 2cm x 2cm from the centre of the palm
                //Ultrahaptics::Vector3 position = device_palm_position + (2 * Ultrahaptics::Units::cm * device_palm_x) + (2 * Ultrahaptics::Units::cm * device_palm_y);
                //Ultrahaptics::Vector3 position = device_palm_position - (2 * Ultrahaptics::Units::cm * device_palm_x) - (2 * Ultrahaptics::Units::cm * device_palm_y);
                //Ultrahaptics::Vector3 position = device_palm_position;

                Leap::Vector boneV = indexFinger.bone(Leap::Bone::Type::TYPE_DISTAL).center();
                Ultrahaptics::Vector3 bone_pos(boneV.x, boneV.y, boneV.z);
                Ultrahaptics::Vector3 position = alignment.fromTrackingPositionToDevicePosition(bone_pos);

                bone_pos.z = bone_pos.z - (10 * Ultrahaptics::Units::mm);

                // Create the control point and emit.
                float strengthFactor = (BUTTON_STRENGTH_BASE + m_buttonStrength) / 10.0f;
                Ultrahaptics::ControlPoint point(position, intensity * strengthFactor, frequency);
                emitter.update(point);
            }
            else
            {
                // We can't see a hand, don't emit anything.
                emitter.stop();
            }
        } else {
            emitter.stop();
        }

        QThread::msleep(10);
    }
}
*/
