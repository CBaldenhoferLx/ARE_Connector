#ifndef ULTRAHAPTICSCONNECTOR_H
#define ULTRAHAPTICSCONNECTOR_H

#include <QObject>
#include <QFuture>

#include "UltrahapticsTimePointStreaming.hpp"
#include <UltrahapticsAmplitudeModulation.hpp>

#include <LeapC++.h>

#include "datareceiver.h"

#define BUTTON_STRENGTH_OFF 0.0
#define BUTTON_STRENGTH_FULL 5.0
#define BUTTON_STRENGTH_BASE 5.0f

using Seconds = std::chrono::duration<float>;
static auto start_time = std::chrono::steady_clock::now();

class UltrahapticsConnector : public DataReceiver
{
    Q_OBJECT
public:
    explicit UltrahapticsConnector(QObject *parent = nullptr);

    void start();

    void stop();

    void setButtonStrength(quint8 strength);

    void setScanEnabled(bool enabled);

    void sendData(SerialProtocol::SerialProtocolAction action);

    static void UltrahapticsConnector::my_emitter_callback(const Ultrahaptics::TimePointStreaming::Emitter &timepoint_emitter,
                             Ultrahaptics::TimePointStreaming::OutputInterval &interval,
                             const Ultrahaptics::HostTimePoint &submission_deadline,
                             void *user_pointer);


    struct Circle
    {
        // The position of the control point
        Ultrahaptics::Vector3 position;

        Ultrahaptics::Vector3 offset;

        // The intensity of the control point
        float intensity;

        // The radius of the circle
        float radius;

        // The frequency at which the control point goes around the circle
        float frequency;

        bool enable;


        const Ultrahaptics::Vector3 evaluateAt(Seconds t){
            // Calculate the x and y positions of the circle and set the height
            position.x = std::cos(2 * M_PI * frequency * t.count()) * radius;
            position.y = std::sin(2 * M_PI * frequency * t.count()) * radius;
        return position;
        }
    };

    // Structure to represent output from the Leap listener
    struct LeapOutput
    {
        LeapOutput() noexcept
        {}

        Ultrahaptics::Vector3 p1;
        Ultrahaptics::Vector3 p2;

        Ultrahaptics::Vector3 palm_position;

        bool hand_present = false;
    };


    // Leap listener class - tracking the hand position and creating data structure for use by Ultrahaptics API
    class LeapListening : public Leap::Listener
    {
    public:
        LeapListening(const Ultrahaptics::Alignment& align, UltrahapticsConnector* c)
          : alignment(align), conn(c)
        {
        }

        ~LeapListening() = default;

        LeapListening(const LeapListening &other) = delete;
        LeapListening &operator=(const LeapListening &other) = delete;

        void onFrame(const Leap::Controller &controller) override
        {
            // Get all the hand positions from the leap and position a focal point on each.
            LeapOutput local_hand_data;

            if (conn->m_buttonStrength!=BUTTON_STRENGTH_OFF && controller.frame().hands().isEmpty()) {
                local_hand_data.palm_position = Ultrahaptics::Vector3();
                local_hand_data.p1 = Ultrahaptics::Vector3();
                local_hand_data.p2 = Ultrahaptics::Vector3();
                local_hand_data.hand_present = false;
            } else {
                Leap::FingerList theFingers = controller.frame().hands().frontmost().fingers().extended();
                Leap::Finger indexFinger;

                for(Leap::FingerList::const_iterator fl = theFingers.begin(); fl != theFingers.end(); fl++) {
                    const Leap::Finger finger = *fl;
                    if (finger.type()==Leap::Finger::Type::TYPE_INDEX) {
                        qDebug() << "Index finger found";
                        indexFinger = finger;
                        break;
                    }
                }

                if (indexFinger.isValid()) {

                } else {
                    local_hand_data.palm_position = Ultrahaptics::Vector3();
                    local_hand_data.p1 = Ultrahaptics::Vector3();
                    local_hand_data.p2 = Ultrahaptics::Vector3();
                    local_hand_data.hand_present = false;
                }


                // Translate the hand position from leap objects to Ultrahaptics objects.
                Leap::Vector leap_palm_position = indexFinger.tipPosition();
                Leap::Vector leap_palm_normal = indexFinger.hand().palmNormal();
                Leap::Vector leap_palm_direction = indexFinger.hand().direction();

                /*
                Leap::Vector leap_palm_position = indexFinger.hand().palmPosition();
                Leap::Vector leap_palm_normal = indexFinger.hand().palmNormal();
                Leap::Vector leap_palm_direction = indexFinger.hand().direction();
                */


                // Convert to Ultrahaptics vectors, normal is negated as leap normal points down.
                Ultrahaptics::Vector3 uh_palm_position(leap_palm_position.x, leap_palm_position.y, leap_palm_position.z);
                Ultrahaptics::Vector3 uh_palm_normal(-leap_palm_normal.x, -leap_palm_normal.y, -leap_palm_normal.z);
                Ultrahaptics::Vector3 uh_palm_direction(leap_palm_direction.x, leap_palm_direction.y, leap_palm_direction.z);

                // Convert from leap space to device space.
                Ultrahaptics::Vector3 device_palm_position = alignment.fromTrackingPositionToDevicePosition(uh_palm_position);
                Ultrahaptics::Vector3 device_palm_normal = alignment.fromTrackingDirectionToDeviceDirection(uh_palm_normal).normalize();
                Ultrahaptics::Vector3 device_palm_direction = alignment.fromTrackingDirectionToDeviceDirection(uh_palm_direction).normalize();

                Leap::Vector boneV1 = indexFinger.bone(Leap::Bone::Type::TYPE_DISTAL).center();
                Ultrahaptics::Vector3 bone_pos1(boneV1.x, boneV1.y, boneV1.z);
                local_hand_data.p1 = alignment.fromTrackingPositionToDevicePosition(bone_pos1);

                Leap::Vector boneV2 = indexFinger.bone(Leap::Bone::Type::TYPE_INTERMEDIATE).center();
                Ultrahaptics::Vector3 bone_pos2(boneV2.x, boneV2.y, boneV2.z);
                local_hand_data.p2 = alignment.fromTrackingPositionToDevicePosition(bone_pos2);

                local_hand_data.palm_position = device_palm_position;

                local_hand_data.hand_present = true;
            }
            atomic_local_hand_data.store(local_hand_data);
        }

        LeapOutput getLeapOutput()
        {
            return atomic_local_hand_data.load();
        }

    private:
        std::atomic<LeapOutput> atomic_local_hand_data;
        Ultrahaptics::Alignment alignment;

        UltrahapticsConnector *conn;
    };


    // Structure for passing information on the type of point to create
    struct ModulatedPoint
    {
        // Create the structure, passing the appropriate alignment through to the LeapListening class
        ModulatedPoint(const Ultrahaptics::Alignment& align, UltrahapticsConnector *c) : hand(align, c)
        {
        }

        // Hand data
        LeapListening hand;

        // The position of the control point
        Ultrahaptics::Vector3 position;

        // The length of the lines to draw
        double line_length;

        // How often the line is drawn every second
        double line_draw_frequency;

        // The offset of the control point at the last sample time
        double offset = 0.0f;

        // This allows us to easily reverse the direction of the point
        // It can be -1 or 1.
        int direction = 1;
    };



private:
    QFuture<void> m_future;

    quint8 m_buttonStrength = BUTTON_STRENGTH_FULL;
    bool m_scanEnabled = false;


private slots:
    void runLoop();

signals:

public slots:
};

#endif // ULTRAHAPTICSCONNECTOR_H
