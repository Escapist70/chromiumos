import os, sys, subprocess, logging
from autotest_lib.client.bin import test, utils
from autotest_lib.client.common_lib import error


class disktest(test.test):
    version = 1

    def setup(self):
        os.chdir(self.bindir)

    def test_one_disk_chunk(self, disk, chunk):
        logging.info("testing %d MB files on %s in %d MB memory",
                     self.chunk_mb, disk, self.memory_mb)
        cmd = "%s/disktest.ia32 -m %d -f %s/testfile.%d -i -S" % \
             (self.bindir, self.chunk_mb, disk, chunk)
        logging.debug(cmd)
        p = subprocess.Popen(cmd, shell=True)
        return(p.pid)

    def execute(self, disks = None, gigabytes = None,
                chunk_mb = utils.memtotal() / 1024):
        if not disks:
            disks = [self.tmpdir]
        if not gigabytes:
            free = 100       # cap it at 100GB by default
            for disk in disks:
                free = min(utils.freespace(disk) / 1024**3, free)
            gigabytes = free
            logging.info("resizing to %s GB", gigabytes)
            sys.stdout.flush()

        self.chunk_mb = chunk_mb
        self.memory_mb = utils.memtotal()/1024
        if self.memory_mb > chunk_mb:
            e_msg = "Too much RAM (%dMB) for this test to work" % self.memory_mb
            raise error.TestError(e_msg)

        chunks = (1024 * gigabytes) / chunk_mb

        for i in range(chunks):
            pids = []
            for disk in disks:
                pid = self.test_one_disk_chunk(disk, i)
                pids.append(pid)
            errors = []
            for pid in pids:
                (junk, retval) = os.waitpid(pid, 0)
                if (retval != 0):
                    errors.append(retval)
            if errors:
                raise error.TestError("Errors from children: %s" % errors)

