#include "caf_config.h"

config::config() {
  opt_group{custom_options_, "global"}
      .add(out_dir, "out_dir", "");
}
