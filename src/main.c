#ifndef WIN32
#define WIN32
#endif

#ifdef WIN32
#include "wingetopt.h"
#else
#include <getopt.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <assert.h>
#include "cmph.h"
#include "hash.h"

#ifdef WIN32
#define VERSION "0.8"
#include <Windows.h>
#else
#include "config.h"
#endif


void usage(const char *prg)
{
	fprintf(stderr, "usage: %s [-v] [-h] [-V] [-k nkeys] [-f hash_function] [-g [-c algorithm_dependent_value][-s seed] ] [-a algorithm] [-M memory_in_MB] [-b algorithm_dependent_value] [-t keys_per_bin] [-d tmp_dir] [-m file.mph]  keysfile\n", prg);
}
void usage_long(const char *prg)
{
	cmph_uint32 i;
	fprintf(stderr, "usage: %s [-v] [-h] [-V] [-k nkeys] [-f hash_function] [-g [-c algorithm_dependent_value][-s seed] ] [-a algorithm] [-M memory_in_MB] [-b algorithm_dependent_value] [-t keys_per_bin] [-d tmp_dir] [-m file.mph] keysfile\n", prg);
	fprintf(stderr, "Minimum perfect hashing tool\n\n");
	fprintf(stderr, "  -h\t print this help message\n");
	fprintf(stderr, "  -c\t c value determines:\n");
	fprintf(stderr, "    \t  * the number of vertices in the graph for the algorithms BMZ and CHM\n");
	fprintf(stderr, "    \t  * the number of bits per key required in the FCH algorithm\n");
	fprintf(stderr, "    \t  * the load factor in the CHD_PH algorithm\n");
	fprintf(stderr, "  -a\t algorithm - valid values are\n");
	for (i = 0; i < CMPH_COUNT; ++i) fprintf(stderr, "    \t  * %s\n", cmph_names[i]);
	fprintf(stderr, "  -f\t hash function (may be used multiple times) - valid values are\n");
	for (i = 0; i < CMPH_HASH_COUNT; ++i) fprintf(stderr, "    \t  * %s\n", cmph_hash_names[i]);
	fprintf(stderr, "  -V\t print version number and exit\n");
	fprintf(stderr, "  -v\t increase verbosity (may be used multiple times)\n");
	fprintf(stderr, "  -k\t number of keys\n");
	fprintf(stderr, "  -g\t generation mode\n");
	fprintf(stderr, "  -s\t random seed\n");
	fprintf(stderr, "  -m\t minimum perfect hash function file \n");
	fprintf(stderr, "  -M\t main memory availability (in MB) used in BRZ algorithm \n");
	fprintf(stderr, "  -d\t temporary directory used in BRZ algorithm \n");
	fprintf(stderr, "  -b\t the meaning of this parameter depends on the algorithm selected in the -a option:\n");
	fprintf(stderr, "    \t  * For BRZ it is used to make the maximal number of keys in a bucket lower than 256.\n");
	fprintf(stderr, "    \t    In this case its value should be an integer in the range [64,175]. Default is 128.\n\n");
	fprintf(stderr, "    \t  * For BDZ it is used to determine the size of some precomputed rank\n");
	fprintf(stderr, "    \t    information and its value should be an integer in the range [3,10]. Default\n");
	fprintf(stderr, "    \t    is 7. The larger is this value, the more compact are the resulting functions\n");
	fprintf(stderr, "    \t    and the slower are them at evaluation time.\n\n");
	fprintf(stderr, "    \t  * For CHD and CHD_PH it is used to set the average number of keys per bucket\n");
	fprintf(stderr, "    \t    and its value should be an integer in the range [1,32]. Default is 4. The\n");
	fprintf(stderr, "    \t    larger is this value, the slower is the construction of the functions.\n");
	fprintf(stderr, "    \t    This parameter has no effect for other algorithms.\n\n");
	fprintf(stderr, "  -t\t set the number of keys per bin for a t-perfect hashing function. A t-perfect\n");
	fprintf(stderr, "    \t hash function allows at most t collisions in a given bin. This parameter applies\n");
	fprintf(stderr, "    \t only to the CHD and CHD_PH algorithms. Its value should be an integer in the\n");
	fprintf(stderr, "    \t range [1,128]. Defaul is 1\n");
	fprintf(stderr, "  keysfile\t line separated file with keys\n");
}

#define ASSERT(Condition) if (!(Condition)) { __debugbreak(); }

int main(int argc, char **argv)
{
	cmph_uint32 verbosity = 0;
	char generate = 0;
	char *mphf_file = NULL;
	FILE *mphf_fd = stdout;
	const char *keys_file = NULL;
	FILE *keys_fd;
	cmph_uint32 nkeys = UINT_MAX;
	cmph_uint32 seed = UINT_MAX;
	CMPH_HASH *hashes = NULL;
	cmph_uint32 nhashes = 0;
	cmph_uint32 i;
	CMPH_ALGO mph_algo = CMPH_CHM;
	double c = 0;
	cmph_config_t *config = NULL;
	cmph_t *mphf = NULL;
	char * tmp_dir = NULL;
	cmph_io_adapter_t *source;
	cmph_uint32 memory_availability = 0;
	cmph_uint32 b = 0;
	cmph_uint32 keys_per_bin = 1;

        //
        // XXX: Begin custom vars.
        //

        BOOL Success;
        //ULONG LastError;
        PVOID BaseAddress;
        HANDLE FileHandle;
        HANDLE MappingHandle;
        //LARGE_INTEGER FileSize;
        LARGE_INTEGER NumberOfElements;
        FILE_STANDARD_INFO FileInfo;

        //
        // XXX: End custom vars.
        //

	while (1)
	{
		char ch = (char)getopt(argc, argv, "hVvgc:k:a:M:b:t:f:m:d:s:");
		if (ch == -1) break;
		switch (ch)
		{
			case 's':
				{
					char *cptr;
					seed = (cmph_uint32)strtoul(optarg, &cptr, 10);
					if(*cptr != 0) {
						fprintf(stderr, "Invalid seed %s\n", optarg);
						exit(1);
					}
				}
				break;
			case 'c':
				{
					char *endptr;
					c = strtod(optarg, &endptr);
					if(*endptr != 0) {
						fprintf(stderr, "Invalid c value %s\n", optarg);
						exit(1);
					}
				}
				break;
			case 'g':
				generate = 1;
				break;
			case 'k':
			        {
					char *endptr;
					nkeys = (cmph_uint32)strtoul(optarg, &endptr, 10);
					if(*endptr != 0) {
						fprintf(stderr, "Invalid number of keys %s\n", optarg);
						exit(1);
					}
				}
				break;
			case 'm':
				mphf_file = strdup(optarg);
				break;
			case 'd':
				tmp_dir = strdup(optarg);
				break;
			case 'M':
				{
					char *cptr;
					memory_availability = (cmph_uint32)strtoul(optarg, &cptr, 10);
					if(*cptr != 0) {
						fprintf(stderr, "Invalid memory availability %s\n", optarg);
						exit(1);
					}
				}
				break;
			case 'b':
				{
					char *cptr;
					b =  (cmph_uint32)strtoul(optarg, &cptr, 10);
					if(*cptr != 0) {
						fprintf(stderr, "Parameter b was not found: %s\n", optarg);
						exit(1);
					}
				}
				break;
			case 't':
				{
					char *cptr;
					keys_per_bin = (cmph_uint32)strtoul(optarg, &cptr, 10);
					if(*cptr != 0) {
						fprintf(stderr, "Parameter t was not found: %s\n", optarg);
						exit(1);
					}
				}
				break;
			case 'v':
				++verbosity;
				break;
			case 'V':
				printf("%s\n", VERSION);
				return 0;
			case 'h':
				usage_long(argv[0]);
				return 0;
			case 'a':
				{
				char valid = 0;
				for (i = 0; i < CMPH_COUNT; ++i)
				{
					if (strcmp(cmph_names[i], optarg) == 0)
					{
						mph_algo = (CMPH_ALGO)i;
						valid = 1;
						break;
					}
				}
				if (!valid)
				{
					fprintf(stderr, "Invalid mph algorithm: %s. It is not available in version %s\n", optarg, VERSION);
					return -1;
				}
				}
				break;
			case 'f':
				{
				char valid = 0;
				for (i = 0; i < CMPH_HASH_COUNT; ++i)
				{
					if (strcmp(cmph_hash_names[i], optarg) == 0)
					{
						hashes = (CMPH_HASH *)realloc(hashes, sizeof(CMPH_HASH) * ( nhashes + 2 ));
						hashes[nhashes] = (CMPH_HASH)i;
						hashes[nhashes + 1] = CMPH_HASH_COUNT;
						++nhashes;
						valid = 1;
						break;
					}
				}
				if (!valid)
				{
					fprintf(stderr, "Invalid hash function: %s\n", optarg);
					return -1;
				}
				}
				break;
			default:
				usage(argv[0]);
				return 1;
		}
	}

	if (optind != argc - 1)
	{
		usage(argv[0]);
		return 1;
	}
	keys_file = argv[optind];

	if (seed == UINT_MAX) seed = (cmph_uint32)time(NULL);
	srand(seed);
	int ret = 0;
	if (mphf_file == NULL)
	{
		mphf_file = (char *)malloc(strlen(keys_file) + 5);
		memcpy(mphf_file, keys_file, strlen(keys_file));
		memcpy(mphf_file + strlen(keys_file), ".mph\0", (size_t)5);
	}

	keys_fd = fopen(keys_file, "r");

	if (keys_fd == NULL)
	{
		fprintf(stderr, "Unable to open file %s: %s\n", keys_file, strerror(errno));
		return -1;
	}

        //
        // XXX: Begin memory map hack.
        //

        FileHandle = CreateFileA(
            keys_file,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
            NULL
        );

        ASSERT(FileHandle && FileHandle != INVALID_HANDLE_VALUE);

        Success = GetFileInformationByHandleEx(
            FileHandle,
            (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
            &FileInfo,
            sizeof(FileInfo)
        );

        if (!Success) {
            fprintf(stderr, "GetFileInformationByHandleEx() failed.\n");
            __debugbreak();
            return -1;
        }

        ASSERT(FileInfo.EndOfFile.QuadPart % 4ULL == 0);

        NumberOfElements.QuadPart = FileInfo.EndOfFile.QuadPart >> 2;

        //
        // Sanity check the number of elements.
        //

        ASSERT(!NumberOfElements.HighPart);

        MappingHandle = CreateFileMappingA(
            FileHandle,
            NULL,
            PAGE_READONLY,
            FileInfo.EndOfFile.HighPart,
            FileInfo.EndOfFile.LowPart,
            NULL
        );

        ASSERT(MappingHandle && MappingHandle != INVALID_HANDLE_VALUE);

        BaseAddress = MapViewOfFile(
            MappingHandle,
            FILE_MAP_READ,
            0,
            0,
            FileInfo.EndOfFile.LowPart
        );

        ASSERT(BaseAddress);

        //
        // XXX: End memory map hack.
        //

        if (seed == UINT_MAX) {
            seed = (cmph_uint32)time(NULL);
        }

        if (nkeys == UINT_MAX) {
            source = cmph_io_nlfile_adapter(keys_fd);
        } else {
            source = cmph_io_nlnkfile_adapter(keys_fd, nkeys);
        }

	if (generate)
	{
		//Create mphf
		mphf_fd = fopen(mphf_file, "w");
		config = cmph_config_new(source);
		cmph_config_set_algo(config, mph_algo);
		if (nhashes) cmph_config_set_hashfuncs(config, hashes);
		cmph_config_set_verbosity(config, verbosity);
		cmph_config_set_tmp_dir(config, (cmph_uint8 *) tmp_dir);
		cmph_config_set_mphf_fd(config, mphf_fd);
		cmph_config_set_memory_availability(config, memory_availability);
		cmph_config_set_b(config, b);
		cmph_config_set_keys_per_bin(config, keys_per_bin);

		//if((mph_algo == CMPH_BMZ || mph_algo == CMPH_BRZ) && c >= 2.0) c=1.15;
		if(mph_algo == CMPH_BMZ  && c >= 2.0) c=1.15;
		if (c != 0) cmph_config_set_graphsize(config, c);

                //
                // XXX: Begin memory map hack.
                //

                cmph_config_set_base_address_and_keylen(config, BaseAddress, sizeof(ULONG));
                source->base_address = BaseAddress;

                //
                // XXX: End memory map hack.
                //

		mphf = cmph_new(config);

		cmph_config_destroy(config);
		if (mphf == NULL)
		{
			fprintf(stderr, "Unable to create minimum perfect hashing function\n");
			//cmph_config_destroy(config);
			free(mphf_file);
			return -1;
		}

		if (mphf_fd == NULL)
		{
			fprintf(stderr, "Unable to open output file %s: %s\n", mphf_file, strerror(errno));
			free(mphf_file);
			return -1;
		}
		cmph_dump(mphf, mphf_fd);
		cmph_destroy(mphf);
		fclose(mphf_fd);
	}
	else
	{
		cmph_uint32 *p;
		cmph_uint32 *hashtable = NULL;
		mphf_fd = fopen(mphf_file, "r");
		if (mphf_fd == NULL)
		{
			fprintf(stderr, "Unable to open input file %s: %s\n", mphf_file, strerror(errno));
			free(mphf_file);
			return -1;
		}
		mphf = cmph_load(mphf_fd);
		fclose(mphf_fd);
		if (!mphf)
		{
			fprintf(stderr, "Unable to parser input file %s\n", mphf_file);
			free(mphf_file);
			return -1;
		}

		cmph_uint32 siz = cmph_size(mphf);
		hashtable = (cmph_uint32 *)calloc(siz, sizeof(cmph_uint32));
		cmph_uint32 *seen = (cmph_uint32*)calloc(siz, sizeof(cmph_uint32));
		cmph_uint32 *seen2 = (cmph_uint32*)calloc(siz, sizeof(cmph_uint32));
		//memset(hashtable, 0,(size_t) siz);
                p = (cmph_uint32 *)BaseAddress;
		//check all keys
		for (i = 0; i < source->nkeys; ++i, ++p)
		{
			cmph_uint32 s, i2, h, h2, ht;
			char *buf;
			cmph_uint32 buflen = sizeof(ULONG);
			//source->read(source->data, &buf, &buflen);
                        buf = (char *)p;

			h2 = cmph_search(mphf, buf, buflen);
                        if (0 && h2 == 6394) {
                            __debugbreak();
                        }
			h = cmph_search(mphf, buf, buflen);
                        s = seen[h];
                        i2 = seen2[h];
                        ht = hashtable[h];
			if (!(h < siz)) {
                                //__debugbreak();
				//fprintf(stderr, "Unknown key %u in the input.\n", *p);
                                fprintf(stderr, "Unknown!  i: %u, v: %u, s: %u, h: %u, ht: %u.\n", i, *p, s, h, ht);
				ret = 1;
                                //break;
			} else if (ht >= keys_per_bin) {
                                fprintf(stderr, "Conflict!  i: %u, v: %u, s: %u, h: %u, ht: %u\n", i, *p, s, h, ht);
                                //__debugbreak();
				//fprintf(stderr, "More than %u keys were mapped to bin %u\n", keys_per_bin, h);
                                //fprintf(stderr, "Duplicated or unknown key %u in the input\n", *p);
				ret = 1;
                                //break;
                        } else {
                            seen[h] = *p;
                            seen2[h] = i;
                        }

                        hashtable[h] = ht + 1;

			if (verbosity)
			{
				printf("%u -> %u\n", *p, h);
			}
			//source->dispose(source->data, buf, buflen);
		}

		cmph_destroy(mphf);
		free(hashtable);
	}
	fclose(keys_fd);
	free(mphf_file);
	free(tmp_dir);
        cmph_io_nlfile_adapter_destroy(source);
	return ret;

}
