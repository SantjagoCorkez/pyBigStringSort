"""
Author: Andrey Semenov, 2022
"""
import os
import sys

from setuptools import setup
from setuptools.extension import Extension

sources = []

with open(os.sep.join([os.path.dirname(os.path.abspath(__file__)), 'CMakeLists.txt']),
          'r',
          encoding='utf-8') as cmake_conf:
    lib_found = False
    list_started = False

    lines = cmake_conf.readlines()
    for i in range(len(lines)):
        line = lines[i].strip()
        if not line:
            continue
        if line.startswith('#'):
            continue

        if list_started:
            if line == ')':
                break
            sources.append(line)
            continue

        if line.startswith('add_library('):
            if lines[i + 1].strip() == 'big_string_sort.${Python3_SOABI}':
                lib_found = True

        if lib_found and line == 'SHARED':
            list_started = True

setup(
    name='big_string_sort',
    author='Andrey Semenov',
    version='1.2',
    description="C implementation of retrieving large sorted byte sequences from disk",
    python_requires=">=3.6",
    license='MIT',
    ext_modules=[
        Extension(
            'big_string_sort',
            sources,
            extra_compile_args=["-Wno-missing-braces",
                                "-fpic",
                                "-fno-common",
                                f"-O{'0' if '-g' in sys.argv else 3}",
                                "-funroll-loops",
                                f"-D_GNU_SOURCE",
                                ],
            extra_link_args=[
                                f"-O{'0' if '-g' in sys.argv else 3}",
                                "-fpic",
                            ],
        ),
    ],
    classifiers=[
        'License :: OSI Approved :: MIT License',
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'Programming Language :: Python :: 3 :: Only',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ],
    data_files=['big_string_sort.pyi']
)
