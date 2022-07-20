/**************************************************************************/ /**
 * @brief Simple SSR-to-OSR utility.
 ******************************************************************************/

#include <functional>
#include <iomanip>
#include <memory>
#include <sstream>
#include <thread>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <point_one/polaris/polaris_client.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "serial_port.h"
#include "osr_producer.h"

////////////////////////////////////////////////////////////////////////////////
// Polaris OSR Corrections Input
////////////////////////////////////////////////////////////////////////////////

DEFINE_bool(polaris_osr, false,
            "Read OSR correction data from a Polaris server. You must provide "
            " Polaris API key.");
DEFINE_string(polaris_osr_hostname, "polaris.pointonenav.com",
              "The hostname of the Polaris server providing OSR corrections.");
DEFINE_string(polaris_osr_api_hostname, "api.pointonenav.com",
              "The hostname of the Polaris API server.");
DEFINE_string(polaris_osr_api_key, "",
              "The API key to use when connecting to Polaris for OSR.");
DEFINE_string(polaris_osr_unique_id, "ssr-to-osr",
              "A unique string to identify this Polaris connection among "
              "others using the same API key.");

////////////////////////////////////////////////////////////////////////////////
// Polaris SSR Corrections Input
////////////////////////////////////////////////////////////////////////////////

DEFINE_bool(polaris_ssr, false,
            "Read SSR correction data from a Polaris server. You must provide "
            " a Polaris beacon ID and a API key.");
DEFINE_string(polaris_ssr_hostname, "ssrz.polaris.p1beta.com",
              "The hostname of the Polaris server providing SSR corrections.");
DEFINE_string(polaris_ssr_api_hostname, "api.p1beta.com",
              "The hostname of the Polaris API server.");
DEFINE_string(polaris_ssr_beacon, "The ID of the beacon providing SSR.", "");
DEFINE_string(polaris_ssr_api_key, "",
              "The API key to use when connecting to Polaris for SSR.");
DEFINE_string(polaris_ssr_unique_id, "ssr-to-osr",
              "A unique string to identify this Polaris SSR connection among "
              "others using the same API key.");

////////////////////////////////////////////////////////////////////////////////
// GNSS Receiver Input/Output
////////////////////////////////////////////////////////////////////////////////

DEFINE_string(
    sbf_path, "/dev/ttyACM0",
    "A path to the serial port connected to the recevier from which to read "
    "SBF messages and to which to write corrections messages. (default: "
    "/dev/ttyACM0)");

DEFINE_uint32(receiver_speed, 460800, "The receiver's serial port's speed.");

DEFINE_string(
    sbf_interface, "USB1",
    "The Septentrio's name for the interface (connection descriptor) connected "
    "to this application through which it will send SBF messages. (COM1, USB2, "
    "etc.)");

DEFINE_bool(
    lband, false,
    "Enable reception of SSR corrections from the recevier's raw L-Band "
    "antenna over a serial port.");

DEFINE_string(
    lband_path, "/dev/ttyACM1",
    "Path to the serial port connected to the recevier from which to read "
    "raw L-band messages. (default: /dev/ttyACM1");

DEFINE_uint32(lband_speed, 460800,
              "The receiver's L-band serial port's speed.");

DEFINE_string(
    lband_interface, "USB2",
    "The Septentrio's name for the interface (connection descriptor) connected "
    "to this application through which it will send raw L-band data. (COM1, "
    "USB2, etc.)");

DEFINE_uint32(
    lband_frequency, 1555492500,
    "The L-band frequency, in Hz. Values should be in the range of 152500000 "
    "to 1559000000 Hz.");

DEFINE_uint32(
    lband_data_rate, 4800,
    "The L-band data rate. Valid values are 600, 1200, 2400, and 4800.");

DEFINE_string(lband_service, "5555",
              "The L-band service ID as a 4-digit hex number.");

DEFINE_string(lband_scramble, "6969",
              "The L-bband scrambling vector, as a 4-digit hex values.");

DEFINE_string(lband_log_path, "",
              "Record bytes received from L-band to a raw log file.");

////////////////////////////////////////////////////////////////////////////////
// SSR->OSR Data Control
////////////////////////////////////////////////////////////////////////////////

DEFINE_uint32(rtcm_msm_type, 4,
              "The type of RTCM MSM messages (1-7) to produce when using SSR "
              "corrections.");

DEFINE_uint32(rtcm_id, 0,
              "The base station ID to use when using SSR corrections.");

DEFINE_uint32(rtcm_position_type, 1005,
              "The type of RTCM position message to generate when using SSR "
              "corrections.");

DEFINE_string(geoid_file, "",
              "The path to a *.pgm file containing geoid data.");

////////////////////////////////////////////////////////////////////////////////
// Misc settings
////////////////////////////////////////////////////////////////////////////////

DEFINE_bool(configure, true, "Enable receiver configuration.");

/******************************************************************************/
using namespace point_one::polaris;
using namespace point_one::applications;

namespace signal_listener {
std::mutex cv_lock;
std::condition_variable cv;

/****************************************************************************/
extern "C" void signal_handler(int signal) {
  LOG(ERROR) << "Caught signal " << strsignal(signal) << " (" << signal << ").";
  // Restore original signal handler, just in case.
  std::signal(signal, SIG_DFL);
  std::unique_lock<std::mutex> _lock(cv_lock);
  cv.notify_one();
}

/****************************************************************************/
void ListenTo(const std::vector<int>& signals) {
  // Install a signal handlers
  for (auto& signal : signals) {
    VLOG(1) << "Listening for " << strsignal(signal) << " (" << signal << ").";
    std::signal(signal, signal_handler);
  }
}

/****************************************************************************/
void Wait() {
  std::unique_lock<std::mutex> _lock(cv_lock);
  cv.wait(_lock);
}
} // namespace signal_listener

/******************************************************************************/
static void ConfigureSeptentrio(SerialPort& port) {
  port.Write("SSSSSSSSSS\r");

  std::ostringstream ss;
  if (FLAGS_lband) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ss.str("");
    ss << "setDataInOut, " << FLAGS_lband_interface << ", , LBandBeam1\r";
    VLOG(1) << "Sending Septentrio command: \"" << ss.str() << "\"";
    port.Write(ss.str());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ss.str("");
    ss << "setLBandBeams, User1, " << FLAGS_lband_frequency << ", baud"
       << FLAGS_lband_data_rate << ", \"Unknown\", \"Unknown\", Enabled\r";
    VLOG(1) << "Sending Septentrio command: \"" << ss.str() << "\"";
    port.Write(ss.str());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ss.str("");
    ss << "setLBandCustomServiceID, \"" << FLAGS_lband_service << "\", \""
       << FLAGS_lband_scramble << "\", off\r";
    VLOG(1) << "Sending Septentrio command: \"" << ss.str() << "\"";
    port.Write(ss.str());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ss.str("");
    ss << "setLBandSelectMode, manual, , ,\r";
    VLOG(1) << "Sending Septentrio command: \"" << ss.str() << "\"";
    port.Write(ss.str());
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  ss.str("");
  ss << "setSBFOutput, Stream2, " << FLAGS_sbf_interface
     << ", GPSNav+GLONav+GALNav+BDSNav+PVTGeodetic, sec1\r";
  VLOG(1) << "Sending Septentrio command: \"" << ss.str() << "\"";
  port.Write(ss.str());

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  ss.str("");
  ss << "setDataInOut, " << FLAGS_sbf_interface << ", , SBF\r";
  VLOG(1) << "Sending Septentrio command: \"" << ss.str() << "\"";
  port.Write(ss.str());
}

/******************************************************************************/
int main(int argc, char* argv[]) {
  FLAGS_logtostderr = true;
  FLAGS_colorlogtostderr = true;
  gflags::SetUsageMessage(
      "Generate RTK corrections data from available OSR and SSR data sources.");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  google::InitGoogleLogging(argv[0]);
  google::SetVLOGLevel("sample_ssr_client", 1);

  LOG(INFO) << "OSR producer version: " << OSRProducer::VERSION_STR;

  struct {
    long sbf_in_bytes = 0;
    long lband_in_bytes = 0;
    long polaris_osr_in_bytes = 0;
    long polaris_ssr_in_bytes = 0;
    long correction_out_bytes = 0;
  } stats;

  // Load geoid data.
  if (FLAGS_geoid_file.empty()) {
    LOG(ERROR) << "No geoid data file.";
    return 1;
  }
  if (!OSRProducer::LoadGeoidData(FLAGS_geoid_file)) {
    LOG(ERROR) << "Unable to load geoid data file \"" << FLAGS_geoid_file
               << "\".";
    return 1;
  }

  // Create the SSR/OSR corrections producer instance.
  // Input from the serial ports happens in the Boost IO thread while input
  // from the Polaris client(s) happens in another thread.
  // So we'll need to lock access to the producer.
  OSRConfiguration config;
  config.rtcm_msm_type_ = FLAGS_rtcm_msm_type;
  config.rtcm_station_id_ = FLAGS_rtcm_id;
  config.rtcm_position_type_ = FLAGS_rtcm_position_type;
  config.receiver_type_ = OSRConfiguration::ReceiverType::SEPTENTRIO_SBF;
  OSRProducer producer(config);
  std::mutex producer_lock;

  // Create a Boost IO service and thread to handle IO for the serial ports
  // and the NTRIP client.
  boost::asio::io_service io_service;
  boost::asio::io_service::work work(io_service);
  std::thread event_loop_thread(
      boost::bind(&boost::asio::io_service::run, &io_service));

  // Open the serial port to the receiver through which we'll send RTCM
  // corrections.
  SerialPort corrections_out_port(&io_service);
  corrections_out_port.Open(FLAGS_sbf_path, FLAGS_receiver_speed);
  producer.SetRTCMCallback([&](const uint8_t* buffer, size_t size_bytes) {
    stats.correction_out_bytes += size_bytes;
    corrections_out_port.Write(buffer, size_bytes);
  });

  // Configure the Septentio to send SBF and raw L-band byte streams.
  if (FLAGS_configure) {
    ConfigureSeptentrio(corrections_out_port);
  }

  // If requested, create a Polaris client for OSR. Pass the corrections it
  // receives over the network to the OSR producer's OSR input.
  std::unique_ptr<point_one::polaris::PolarisClient> polaris_osr_client;
  if (FLAGS_polaris_osr) {
    if (FLAGS_polaris_osr_api_key.empty()) {
      LOG(ERROR) << "Please provide a Polaris OSR API key.";
      return 1;
    }
    polaris_osr_client.reset(new PolarisClient(FLAGS_polaris_osr_api_key,
                                               FLAGS_polaris_osr_unique_id));
    if (!FLAGS_polaris_osr_api_hostname.empty()) {
      polaris_osr_client->SetPolarisAuthenticationServer(
          FLAGS_polaris_osr_api_hostname);
    }
    if (!FLAGS_polaris_osr_hostname.empty()) {
      polaris_osr_client->SetPolarisEndpoint(FLAGS_polaris_osr_hostname);
    }
    polaris_osr_client->SetRTCMCallback(
        [&](const uint8_t* buffer, size_t size_bytes) {
          stats.polaris_osr_in_bytes += size_bytes;
          std::unique_lock<std::mutex> lock(producer_lock);
          producer.HandleOSR(buffer, size_bytes);
        });
    polaris_osr_client->RunAsync();
  }

  // If requested, create a Polaris client for SSR. Pass the corrections it
  // receives over the network to the OSR producer's SSR input.
  std::unique_ptr<point_one::polaris::PolarisClient> polaris_ssr_client;
  if (FLAGS_polaris_ssr) {
    if (FLAGS_polaris_ssr_api_key.empty()) {
      LOG(ERROR) << "Please provide a Polaris SSR API key.";
      return 1;
    }
    polaris_ssr_client.reset(new PolarisClient(FLAGS_polaris_ssr_api_key,
                                               FLAGS_polaris_ssr_unique_id));
    if (!FLAGS_polaris_ssr_api_hostname.empty()) {
      polaris_ssr_client->SetPolarisAuthenticationServer(
          FLAGS_polaris_ssr_api_hostname);
    }
    if (!FLAGS_polaris_ssr_hostname.empty()) {
      polaris_ssr_client->SetPolarisEndpoint(FLAGS_polaris_ssr_hostname);
    }
    if (FLAGS_polaris_ssr_beacon.empty()) {
      LOG(ERROR) << "Please provide a Polaris SSR beacon ID.";
      return 1;
    }
    polaris_ssr_client->RequestBeacon(FLAGS_polaris_ssr_beacon);
    polaris_ssr_client->SetRTCMCallback(
        [&](const uint8_t* buffer, size_t size_bytes) {
          stats.polaris_ssr_in_bytes += size_bytes;
          std::unique_lock<std::mutex> lock(producer_lock);
          producer.HandleSSR(buffer, size_bytes);
        });
    polaris_ssr_client->RunAsync();
  }

  // Hook into the OSR producer's SetPositionTimeCallback so that when it
  // receives a PVTGeodetic SBF message we can pass our location to the Polaris
  // OSR client.
  double last_position_time_seconds = 0;
  int last_week = 0;
  auto position_updater = [&](int week, double time_of_week_secs,
                              const std::array<double, 3>& lla_deg) {
    if (0 == week || std::isnan(time_of_week_secs)) {
      return;
    }
    // Limit position updates to not more frequent than once every 30s.
    if (last_week == week &&
        time_of_week_secs < last_position_time_seconds + 30.0) {
      return;
    }
    last_position_time_seconds = time_of_week_secs;
    last_week = week;
    if (0.0 == last_position_time_seconds) {
      LOG(INFO) << "Initial position set at (" << lla_deg[0] << ", "
                << lla_deg[1] << ", " << lla_deg[2] << ").";
    }
    if (polaris_osr_client) {
      polaris_osr_client->SendLLAPosition(lla_deg[0], lla_deg[1], lla_deg[2]);
    }
  };
  producer.SetPositionTimeCallback(position_updater);

  // Open a serial port from which to read the receiver's raw L-band messages.
  // Pass these messages to the OSR producer's secondary SSR input.
  SerialPort lband_port(&io_service);
  int lband_log_fd = -1;
  if (FLAGS_lband) {
    if (!FLAGS_lband_log_path.empty()) {
      lband_log_fd = open(FLAGS_lband_log_path.c_str(),
                          O_CREAT | O_WRONLY | O_TRUNC, 0666);
      if (-1 == lband_log_fd) {
        LOG(ERROR) << "Unable to open \"" << FLAGS_lband_log_path
                   << "\": " << strerror(errno);
        return 1;
      }
    }
    lband_port.Open(FLAGS_lband_path, FLAGS_lband_speed,
                    [&](const uint8_t* data, size_t size_bytes) {
                      stats.lband_in_bytes += size_bytes;
                      if (-1 != lband_log_fd) {
                        write(lband_log_fd, data, size_bytes);
                      }
                      std::unique_lock<std::mutex> lock(producer_lock);
                      producer.HandleSecondarySSR(data, size_bytes);
                    });
  }

  // Open a serial port from which to read the Septentrio's SBF messages.
  // Pass these mesasges to the OSR producer via its receiver data input.
  SerialPort sbf_port(&io_service);
  sbf_port.Open(FLAGS_sbf_path, FLAGS_receiver_speed,
                [&](const uint8_t* data, size_t size_bytes) {
                  stats.sbf_in_bytes += size_bytes;
                  std::unique_lock<std::mutex> lock(producer_lock);
                  producer.HandleReceiverData(data, size_bytes);
                });

  signal_listener::ListenTo({SIGABRT, SIGINT, SIGTERM});
  signal_listener::Wait();

  LOG(INFO) << "Shutting down.";

  sbf_port.Close();

  lband_port.Close();

  if (-1 != lband_log_fd) {
    close(lband_log_fd);
  }

  polaris_ssr_client.reset();

  polaris_osr_client.reset();

  corrections_out_port.Close();

  io_service.stop();
  event_loop_thread.join();

  LOG(INFO) << "IO Stats:";
  LOG(INFO) << std::setw(12) << std::setfill(' ') << stats.sbf_in_bytes
            << "  SBF bytes read from receiver";
  LOG(INFO) << std::setw(12) << std::setfill(' ') << stats.lband_in_bytes
            << "  L-band SSR bytes read from receiver";
  LOG(INFO) << std::setw(12) << std::setfill(' ') << stats.polaris_osr_in_bytes
            << "  OSR bytes read from Polaris server";
  LOG(INFO) << std::setw(12) << std::setfill(' ') << stats.polaris_ssr_in_bytes
            << "  SSR bytes read from Polaris server";
  LOG(INFO) << std::setw(12) << std::setfill(' ') << stats.correction_out_bytes
            << "  Correction OSR bytes written to receiver";

  return 0;
}
