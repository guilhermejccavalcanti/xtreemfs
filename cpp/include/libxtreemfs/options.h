/*
 * Copyright (c) 2010-2011 by Patrick Schaefer, Zuse Institute Berlin
 *                    2011 by Michael Berlin, Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

#ifndef CPP_INCLUDE_LIBXTREEMFS_OPTIONS_H_
#define CPP_INCLUDE_LIBXTREEMFS_OPTIONS_H_

#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <vector>

#include "libxtreemfs/user_mapping.h"

namespace xtreemfs {

namespace rpc {
class SSLOptions;
}  // namespace rpc

enum XtreemFSServiceType {
  kDIR, kMRC
};

class Options {
 public:
  /** Sets the default values. */
  Options();

  virtual ~Options() {}

  /** Generates boost::program_options description texts. */
  void GenerateProgramOptionsDescriptions();

  /** Set options parsed from command line.
   *
   * However, it does not set dir_volume_url and does not call
   * ParseVolumeAndDir().
   *
   * @throws InvalidCommandLineParametersException
   * @throws InvalidURLException */
  std::vector<std::string> ParseCommandLine(int argc, char** argv);

  /** Outputs usage of the command line parameters of all options. */
  virtual std::string ShowCommandLineHelp();

  /** Outputs usage of the command line parameters of volume creation
   *  relevant options. */
  std::string ShowCommandLineHelpVolumeCreationAndDeletion();

  /** Outputs usage of the command line parameters of volume deletion/listing
   *  relevant options. */
  std::string ShowCommandLineHelpVolumeListing();

  /** Returns the version string and prepends "component". */
  std::string ShowVersion(const std::string& component);

  /** Returns true if required SSL options are set. */
  bool SSLEnabled() const;

  /** Creates a new SSLOptions object based on the value of the members:
   *  - ssl_pem_key_path
   *  - ssl_pem_cert_path
   *  - ssl_pem_key_pass
   *  - ssl_pkcs12_path
   *  - ssl_pkcs12_pass
   *  - grid_ssl || protocol.
   *
   * @remark Ownership is transferred to caller. May be NULL.
   */
  xtreemfs::rpc::SSLOptions* GenerateSSLOptions() const;

  // Version information.
  std::string version_string;

  // XtreemFS URL Options.
  /** URL to the Volume, Form: [pbrpc://]service-hostname[:port]/volume_name.  // NOLINT
   *
   * Depending on the type of operation the service-hostname has to point to the
   * DIR (to open/"mount" a volume) or the MRC (create/delete/list volumes).
   * Depending on this type, the default port differs (DIR: 32638; MRC: 32636).
   */
  std::string xtreemfs_url;
  /** Usually extracted from xtreemfs_url (Form: ip-address:port). */
  std::string service_address;
  /** Usually extracted from xtreemfs_url. */
  std::string volume_name;
  /** Usually extracted from xtreemfs_url. */
  std::string protocol;
  /** Mount point on local system (set by ParseCommandLine()). */
  std::string mount_point;

  // General options.
  /** Log level as string (EMERG|ALERT|CRIT|ERR|WARNING|NOTICE|INFO|DEBUG). */
  std::string log_level_string;
  /** If not empty, the output will be logged to a file. */
  std::string log_file_path;
  /** True, if "-h" was specified. */
  bool show_help;
  /** True, if argc == 1 was at ParseCommandLine(). */
  bool empty_arguments_list;
  /** True, if -V/--version was specified and the version will be shown only .*/
  bool show_version;

  // Optimizations.
  /** Maximum number of entries of the StatCache */
  boost::uint64_t metadata_cache_size;
  /** Time to live for MetadataCache entries. */
  boost::uint64_t metadata_cache_ttl_s;
  /** Maximum number of pending bytes (of async writes) per file. */
  int max_writeahead;
  /** Maximum number of pending async write requests per file. */
  int max_writeahead_requests;
  /** Number of retrieved entries per readdir request. */
  int readdir_chunk_size;

  // Error Handling options.
  /** How often shall a failed operation get retried? */
  int max_tries;
  /** How often shall a failed read operation get retried? */
  int max_read_tries;
  /** How often shall a failed write operation get retried? */
  int max_write_tries;
  /** How long to wait after a failed request? */
  int retry_delay_s;
  /** Maximum time until a connection attempt will be aborted. */
  boost::int32_t connect_timeout_s;
  /** Maximum time until a request will be aborted and the response returned. */
  boost::int32_t request_timeout_s;
  /** The RPC Client closes connections after "linger_timeout_s" time of
   *  inactivity. */
  boost::int32_t linger_timeout_s;

  // SSL options.
  std::string ssl_pem_cert_path;
  std::string ssl_pem_key_path;
  std::string ssl_pem_key_pass;
  std::string ssl_pkcs12_path;
  std::string ssl_pkcs12_pass;

  // Grid Support options.
  /** True, if the XtreemFS Grid-SSL Mode (only SSL handshake, no encryption of
   *  data itself) shall be used. */
  bool grid_ssl;
  /** True if the Globus user mapping shall be used. */
#ifndef WIN32
  bool grid_auth_mode_globus;
  /** True if the Unicore user mapping shall be used. */
  bool grid_auth_mode_unicore;
  /** Location of the gridmap file. */
  std::string grid_gridmap_location;
  /** Default Location of the Globus gridmap file. */
  std::string grid_gridmap_location_default_globus;
  /** Default Location of the Unicore gridmap file. */
  std::string grid_gridmap_location_default_unicore;
  /** Periodic interval after which the gridmap file will be reloaded. */
  int grid_gridmap_reload_interval_m;
#endif  // !WIN32

  // Vivaldi Options
  /** Enables the vivaldi coordinate calculation for the client. */
  bool vivaldi_enable;
  /** Enables sending the coordinates to the DIR after each recalculation. This
   *  is only needed to add the clients to the vivaldi visualisation at the cost
   *  of some additional traffic between client and DIR.") */
  bool vivaldi_enable_dir_updates;
  /** The file where the vivaldi coordinates should be saved after each
   *  recalculation. */
  std::string vivaldi_filename;
  /** The interval between coordinate recalculations. Also see
   *  vivaldi_recalculation_epsilon_s. */
  unsigned int vivaldi_recalculation_interval_s;
  /** The recalculation interval will be randomly chosen from
   *  vivaldi_recalculation_inverval_s +/- vivaldi_recalculation_epsilon_s */
  unsigned int vivaldi_recalculation_epsilon_s;
  /** Number of coordinate recalculations before updating the list of OSDs. */
  unsigned int vivaldi_max_iterations_before_updating;
  /** Maximal number of retries when requesting coordinates from another
   *  vivaldi node. */
  unsigned int vivaldi_max_request_retries;

  // Advanced XtreemFS options.
  /** Interval for periodic file size updates in seconds. */
  int periodic_file_size_updates_interval_s;
  /** Interval for periodic xcap renewal in seconds. */
  int periodic_xcap_renewal_interval_s;
  /** Skewness of the Zipf distribution used for vivaldi OSD selection */
  double vivaldi_zipf_generator_skew;

  // Deprecated options
  /** Stops retrying to execute a synchronous request if this signal was send
   *  to the thread responsible for the execution of the request. */
  int interrupt_signal;

#ifndef WIN32
  // User mapping.
  /** Type of the UserMapping used to resolve user and group IDs to names. */
  UserMapping::UserMappingType user_mapping_type;
#endif  // !WIN32

 protected:
  /** Extract volume name and dir service address from dir_volume_url. */
  void ParseURL(XtreemFSServiceType service_type);

 private:
  /** Reads password from stdin and stores it in 'password'. */
  void ReadPasswordFromStdin(const std::string& msg, std::string* password);

  /** This functor template can be used as argument for the notifier method of
   *  boost::options. It is specifically used to create a warning whenever
   *  a deprecated option is used, but is not limited to that purpose.
   *  Use the createOptionHandler function template to instantiate it (it uses
   *  a value for type inference).
   *
   */
  template<typename T>
  class OptionHandler {
   public:
    typedef void result_type;
    OptionHandler(std::string msg)  // first parameter is for type inference
     : msg_(msg) { }

    void operator()(const T& value) {
      std::cout << "Warning: Deprecated option used: " << msg_ << std::endl;  // TODO(mno): maybe output somewhere else?!
    }

   private:
    const std::string msg_;
  };

  /** See OptionHandler */
  template<typename T>
  OptionHandler<T> createOptionHandler(const T&, std::string msg) {
    return OptionHandler<T>(msg);
  }

  // Sums of options.
  /** Contains all boost program options, needed for parsing and by
   *  ShowCommandLineHelp(). */
  boost::program_options::options_description all_descriptions_;

  /** Contains descriptions of advanced options. */
  boost::program_options::options_description hidden_descriptions_;

  /** Set to true if GenerateProgramOptionsDescriptions() was executed. */
  bool all_descriptions_initialized_;

  // Options itself.
  /** Description of general options (Logging, help). */
  boost::program_options::options_description general_;

  /** Description of options which improve performance. */
  boost::program_options::options_description optimizations_;

  /** Descrption of timeout options etc. */
  boost::program_options::options_description error_handling_;

  /** Description of SSL related options. */
  boost::program_options::options_description ssl_options_;

  /** Description of options of the Grid support. */
  boost::program_options::options_description grid_options_;

  /** Description of the vivaldi options */
  boost::program_options::options_description vivaldi_options_;

  // Hidden options.
  /** Description of options of the Grid support. */
  boost::program_options::options_description xtreemfs_advanced_options_;

  /** Deprecated options which are kept to ensure backward compatibility. */
  boost::program_options::options_description deprecated_options_;
};

}  // namespace xtreemfs

#endif  // CPP_INCLUDE_LIBXTREEMFS_OPTIONS_H_
