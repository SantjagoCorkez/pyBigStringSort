from os import PathLike


class BigStringSort:
    @staticmethod
    def sorted(path: PathLike) -> bytes:
        """
        Reads the file pointed to by the `path` parameter and returns it sorted byte-wise.
        Possible exceptions being raised:
         - PyUnicode_AsUTF8 could raise UnicodeEncodeError if a path given contains invalid Unicode points
          (the doc does not specify the certain errors being raised)
         - OSError - a file-related syscall operation failed (fopen()/fclose()/stat()); errno is then provided within the exception raised
         - OSError - a memory-related call (malloc()) failed; errno is then passed through to the exception
         - OSError - a call to nanosleep() failed in a strange and unrecoverable manner (shouldn't occur still the code should handle this)
         - OverflowError - the file pointed to is larger that the largest memory region the runtime could handle
         - MemoryError - the runtime was unable to allocate enough memory to store the result
         - BrokenPipeError - file modification (size decreased) detected while the file was being read
         - IOError - file read failed even after 10 recovery attempts (disk damaged, network error for remote FSes, ...)

        :param path: path to a file; is used in system `fopen()` as is.
         If it's not an absolute path it's caller's responsibility to ensure the file is available to the runtime by its
         relative path
        :return: the bytes sequence already sorted (according to characters' integer values)
        """
