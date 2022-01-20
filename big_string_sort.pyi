from os import PathLike


class BigStringSort:
    @staticmethod
    def sorted(path: PathLike) -> bytes:
        """
        Reads the file pointed to by the `path` parameter and returns it sorted byte-wise.
        Possible exceptions being raised:
         - OSError - a file-related syscall operation failed (fopen()/fclose()); errno is then provided within the exception raised
         - OverflowError - the file pointed to is larger that the largest memory region the runtime could handle
         - MemoryError - the runtime was unable to allocate enough memory to store the result

        :param path: path to a file; is used in system `fopen()` as is.
         If it's not an absolute path it's caller's responsibility to ensure the file is available to the runtime by its
         relative path
        :return: the bytes sequence already sorted (according to characters' integer values)
        """
