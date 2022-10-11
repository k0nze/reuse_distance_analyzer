from glob import glob
import os
import sys
import subprocess
from pathlib import Path

import distutils

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

from setuptools.command.develop import develop as develop_orig
from setuptools.command.install import install as install_orig

from shutil import which


class develop(develop_orig):

    def run(self):
        print("editable install (entry point)")
        global editable_install
        editable_install = True
        super().run()


class install(install_orig):

    def run(self):
        print("non-editable install (entry point)")
        super().run()


class CMakeExtension(Extension):

    def __init__(self, name):
        Extension.__init__(self, name, sources=[])


class CMakeBuild(build_ext):

    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: " +
                ", ".join(e.name for e in self.extensions))

        build_directory = os.path.abspath(self.build_temp)

        if "editable_wheel" in sys.argv:
            global editable_install
            editable_install = True

        # get path to directory where the reusedist_backend will be after build
        if 'editable_install' in globals():
            print("editable install")
            self.reusedist_backend_dir_path = Path(
                self.get_ext_fullpath('reusedist')).resolve()
            self.reusedist_backend_dir_path = Path(
                str(self.reusedist_backend_dir_path.parents[0]) +
                "/reusedist/backend/")
        else:
            print("non-editable install")
            self.reusedist_backend_dir_path = Path(
                distutils.sysconfig.get_python_lib() + "/reusedist/backend/")

        print(f"Path to REUSEDIST backend: {self.reusedist_backend_dir_path}")

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={build_directory}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DPYBIND11_PYTHON_VERSION={sys.version_info.major}.{sys.version_info.minor}",
            f"-DBUILD_ACADL=OFF", f"-DBUILD_ASWDL=OFF",
            f"-DMOVE_LIBREUSEDIST=ON",
            f"-DLIBREUSEDIST_OUTPUT_DIRECTORY={self.reusedist_backend_dir_path}/"
        ]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        cmake_args += [f"-DCMAKE_BUILD_TYPE={cfg}"]

        # check if ninja is installed
        if which("ninja") is not None:
            cmake_args += ['-GNinja']
        else:
            # use Makefile instead
            build_args += ['--', '-j2']

        self.build_args = build_args

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(
            env.get('CXXFLAGS', ''), self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        # CMakeLists.txt is in the same directory as this setup.py file
        cmake_list_dir = os.path.abspath(os.path.dirname(__file__))
        print('-' * 10, 'Running CMake prepare', '-' * 40)
        subprocess.check_call(['cmake', cmake_list_dir] + cmake_args,
                              cwd=self.build_temp,
                              env=env)

        print('-' * 10, 'Building extensions', '-' * 40)
        cmake_cmd = ['cmake', '--build', '.'] + self.build_args
        subprocess.check_call(cmake_cmd, cwd=self.build_temp)

        # Move from build temp to final position
        for ext in self.extensions:
            self.move_output(ext)

    def move_output(self, ext):
        # move reusedist.cpython.*.so to reusedist/backend dir
        build_temp = Path(self.build_temp).resolve()
        source_path = build_temp / self.get_ext_filename(ext.name)
        dest_path = Path(self.get_ext_fullpath(ext.name)).resolve()
        dest_path = Path(
            str(self.reusedist_backend_dir_path) + "/" + dest_path.name)
        dest_directory = dest_path.parents[0]
        dest_directory.mkdir(parents=True, exist_ok=True)
        self.copy_file(source_path, dest_path)


ext_modules = [
    CMakeExtension('reusedist'),
]

setup(
    name="reusedist",
    version="1.0.0",
    packages=["reusedist", "reusedist.utils", "reusedist.backend"],
    package_dir={
        "reusedist": "reusedist",
        "reusedist.utils": "reusedist/utils",
        "reusedist.backend": "reusedist/backend"
    },
    ext_modules=ext_modules,
    cmdclass={
        'develop': develop,
        'install': install,
        'build_ext': CMakeBuild
    },
    zip_safe=False,
)
