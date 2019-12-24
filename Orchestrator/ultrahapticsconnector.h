#ifndef ULTRAHAPTICSCONNECTOR_H
#define ULTRAHAPTICSCONNECTOR_H

#include <QObject>
#include <QFuture>

#include "UltrahapticsTimePointStreaming.hpp"
#include <UltrahapticsAmplitudeModulation.hpp>

#include <LeapC++.h>
#include <atomic>

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

    static void circle_emitter_callback(const Ultrahaptics::TimePointStreaming::Emitter &timepoint_emitter,
                             Ultrahaptics::TimePointStreaming::OutputInterval &interval,
                             const Ultrahaptics::HostTimePoint &submission_deadline,
                             void *user_pointer);

    static void hand_scan_emitter_callback(const Ultrahaptics::TimePointStreaming::Emitter &timepoint_emitter,
                             Ultrahaptics::TimePointStreaming::OutputInterval &interval,
                             const Ultrahaptics::HostTimePoint &submission_deadline,
                             void *user_pointer);

    struct Circle
    {
        // The position of the control point
        Ultrahaptics::Vector3 position = Ultrahaptics::Vector3(0, 0, 0);

        Ultrahaptics::Vector3 offset;

        // The intensity of the control point
        float intensity = 1.0f;

        // The radius of the circle
        float radius = 2.0f * Ultrahaptics::Units::centimetres;

        // The frequency at which the control point goes around the circle
        float frequency = 100.f;

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
      Ultrahaptics::Vector3 palm_position;
      Ultrahaptics::Vector3 x_axis;
      bool hand_present;
    };

    // Leap listener class - tracking the hand position and creating data structure for use by Ultrahaptics API
    class LeapListening : public Leap::Listener
    {
      public:
        LeapListening()
          : current_update(0),
          atomic_local_hand_data(local_hand_data[0])
        {
          current_update++;
        }

        LeapListening(const LeapListening &other) = delete;
        LeapListening& operator=(const LeapListening &other) = delete;

        void onFrame(const Leap::Controller & controller)
        {
          // Get all the hand positions from the leap and position a focal point on each.
          const Leap::Frame    frame = controller.frame();
          const Leap::HandList hands = frame.hands();

          if (hands.isEmpty()) {
            local_hand_data[current_update & 1].palm_position = Ultrahaptics::Vector3();
            local_hand_data[current_update & 1].x_axis = Ultrahaptics::Vector3();
            local_hand_data[current_update & 1].hand_present = false;
            atomic_local_hand_data.store(local_hand_data[current_update & 1]);
            current_update++;

          } else {
            const Leap::Hand & hand = hands[0];

            // Translate the hand position from leap objects to Ultrahaptics objects.
            const Leap::Vector & leap_palm_position  = hand.palmPosition();
            const Leap::Vector & leap_palm_normal    = hand.palmNormal();
            const Leap::Vector & leap_palm_direction = hand.direction();

            // Convert to Ultrahaptics vectors, normal is negated as leap normal points down.
            const Ultrahaptics::Vector3 uh_palm_position  = +Ultrahaptics::Vector3(leap_palm_position.x,  leap_palm_position.y,  leap_palm_position.z);
            const Ultrahaptics::Vector3 uh_palm_normal    = -Ultrahaptics::Vector3(leap_palm_normal.x,    leap_palm_normal.y,    leap_palm_normal.z);
            const Ultrahaptics::Vector3 uh_palm_direction = +Ultrahaptics::Vector3(leap_palm_direction.x, leap_palm_direction.y, leap_palm_direction.z);

            // Convert from leap space to device space.
            const Ultrahaptics::Vector3 device_palm_position  = alignment.fromTrackingPositionToDevicePosition(uh_palm_position);
            const Ultrahaptics::Vector3 device_palm_normal    = alignment.fromTrackingDirectionToDeviceDirection(uh_palm_normal).normalize();
            const Ultrahaptics::Vector3 device_palm_direction = alignment.fromTrackingDirectionToDeviceDirection(uh_palm_direction).normalize();

            // Only respond to the hand when it is roughly in the desired position.
            // This position should correspond to the 'plate' the user puts the hand into the start the experience
            if(  device_palm_position.x < (-4.f * Ultrahaptics::Units::cm)
              || device_palm_position.x > ( 4.f * Ultrahaptics::Units::cm)
              || device_palm_position.y < (-4.f * Ultrahaptics::Units::cm)
              || device_palm_position.y > ( 4.f * Ultrahaptics::Units::cm)
              || device_palm_position.z < (-16.f * Ultrahaptics::Units::cm)
              || device_palm_position.z > ( 24.f * Ultrahaptics::Units::cm))
            {
               local_hand_data[current_update & 1].palm_position = Ultrahaptics::Vector3();
               local_hand_data[current_update & 1].x_axis = Ultrahaptics::Vector3();
               local_hand_data[current_update & 1].hand_present = false;
               atomic_local_hand_data.store(local_hand_data[current_update & 1]);
               current_update++;
               return;
            }

            // These can then be converted to be a unit axis on the palm of the hand.
            const Ultrahaptics::Vector3 device_palm_z = device_palm_normal;                             // Unit Z direction.
            const Ultrahaptics::Vector3 device_palm_y = device_palm_direction;                          // Unit Y direction.
            const Ultrahaptics::Vector3 device_palm_x = device_palm_y.cross(device_palm_z).normalize(); // Unit X direction.

            local_hand_data[current_update & 1].palm_position = device_palm_position;
            local_hand_data[current_update & 1].x_axis = device_palm_x;
            local_hand_data[current_update & 1].hand_present = true;
            atomic_local_hand_data.store(local_hand_data[current_update & 1]);
            current_update++;
          }
        }

        const Ultrahaptics::Vector3 getPalmPosition() { return (atomic_local_hand_data.load()).palm_position; }
        const Ultrahaptics::Vector3 getXAxis() { return (atomic_local_hand_data.load()).x_axis; }
        const bool isHandPresent() { return (atomic_local_hand_data.load()).hand_present; }

      private:
        std::atomic<LeapOutput> atomic_local_hand_data;
        LeapOutput local_hand_data[2];
        Ultrahaptics::Alignment alignment;
        int current_update;
    };


    struct HandScan
    {
       // Hand data
       LeapListening hand;

       // The position of the control point
       Ultrahaptics::Vector3 position = Ultrahaptics::Vector3(0, 0, 0);

       // The intensity of the control point
       float intensity = 1.f;

       // The width and height of the scan area
       float forcefield_width = 8.f * Ultrahaptics::Units::centimetres;
       float forcefield_height = 12.f * Ultrahaptics::Units::centimetres;

       // The frequency of forcefield update. Y = one vertical 'scan' takes 1.5 seconds
       float forcefield_frequency_x = 80.f;
       float forcefield_frequency_y = 1.5f;

       // The offset of the control point at the last sample time
       float previous_sample_offset_x;
       float previous_sample_offset_y;

       // Is this the initial interval?
       bool is_initial_interval = true;

       // This allows us to easily reverse the direction of the point
       bool direction_x = true;
       bool direction_y = true;
    };


private:
    QFuture<void> m_future;

    quint8 m_buttonStrength = BUTTON_STRENGTH_FULL;
    bool m_scanEnabled = true;
    Ultrahaptics::TimePointStreaming::Emitter *m_emitter;
    HandScan m_hand_scan_data;
    Circle m_circle_data;


private slots:
    void runLoop();

signals:

public slots:
};

#endif // ULTRAHAPTICSCONNECTOR_H
