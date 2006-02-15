#define APT_BUILD_CONF_PATH "/etc/apt/apt-build.conf"
#define APT_BUILD_STRING_ENV "APT_BUILD_WRAPPER"

#define LIBDIR "/usr/lib/apt-build"

#define MAKE_ARGC 0
#define GCC_ARGC 2

#define BUF_SIZE 1024

struct apt_build_args
{
  char *Olevel;
  char *mtune;
  char *options;
  char *make_options;
};
