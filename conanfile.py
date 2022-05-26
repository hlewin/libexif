from conans import ConanFile, AutoToolsBuildEnvironment, tools
from conans.errors import ConanInvalidConfiguration
import os
from conan.tools.files import replace_in_file

required_conan_version = ">=1.33.0"


class LibexifConan(ConanFile):
    name = "libexif"
    version = "0.6.24"
    url = "https://gitlab.worldiety.net/worldiety/customer/wdy/libriety/cpp/forks"
    homepage = "https://libexif.github.io/"
    license = "LGPL-2.1"
    description = "libexif is a library for parsing, editing, and saving EXIF data."
    topics = ("exif", "metadata", "parse", "edit")

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
    }
    _ndk_dep = "android-ndk/[]"
    
    exports_sources = "*", "!autom4te.cache"
    python_requires = "wdyConanHelper/[]"
    python_requires_extend = "wdyConanHelper.ConanAutotools"

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd


    def configure_args(self):
        yes_no = lambda v: "yes" if v else "no"
        args = [
            "--enable-shared={}".format(yes_no(self.options.shared)),
            "--enable-static={}".format(yes_no(not self.options.shared)),
            "--disable-largefile",
            "--disable-nls"
        ]
        return args
        
    def configure_env(self, at):
        at.flags.append("-fexceptions")

