#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <librpmem.h>

#define POOL_SIZE	(32 * 1024 * 1024)
#define NLANES		4

int
main(int argc, char *argv[])
{
	int ret;
	unsigned nlanes = NLANES;

	/* allocate a page size aligned local memory pool */
	long pagesize = sysconf(_SC_PAGESIZE);
	void *pool;
	ret = posix_memalign(&pool, pagesize, POOL_SIZE);
	if (ret) {
		fprintf(stderr, "posix_memaling: %s\n", strerror(ret));
		return 1;
	}
	assert(pool != NULL);

	/* fill pool_attributes */
	struct rpmem_pool_attr pool_attr;
	memset(&pool_attr, 0, sizeof(pool_attr));

	/* create a remote pool */
	RPMEMpool *rpp = rpmem_create("localhost", "pool.set",
		pool, POOL_SIZE, &nlanes, &pool_attr);
	if (!rpp) {
		fprintf(stderr, "rpmem_create: %s\n", rpmem_errormsg());
		return 1;
	}

	/* store data in the local pool */
	memset(pool, 0, POOL_SIZE);

	/* make local data persistent on the target node */
	ret = rpmem_persist(rpp, 0, POOL_SIZE, 0, 0);
	if (ret) {
		fprintf(stderr, "rpmem_persist: %s\n", rpmem_errormsg());
		return 1;
	}

	/* close the remote pool */
	ret = rpmem_close(rpp);
	if (ret) {
		fprintf(stderr, "rpmem_close: %s\n", rpmem_errormsg());
		return 1;
	}

	/* release the local memory pool */
	free(pool);

	return 0;
}
