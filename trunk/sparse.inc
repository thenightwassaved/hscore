
/* section: typedefs */
typedef unsigned char sparse_chunk_1_t[8][8];
typedef sparse_chunk_1_t *sparse_chunk_2_t[4][4];
typedef sparse_chunk_2_t *sparse_chunk_3_t[32][32];
typedef sparse_chunk_3_t *sparse_arr;

/* section: allocate memory/initialize */

static sparse_arr init_sparse(void)
{
	int x, y;
	sparse_arr c;

	c = amalloc(sizeof(sparse_chunk_3_t));
	if (c == NULL)
		return NULL;

	for (x = 0; x < 32; x++)
		for (y = 0; y < 32; y++)
			(*c)[x][y] = NULL;
	return c;
}

/* section: release memory */

static void delete_sparse(sparse_arr c_3)
{
	int x, y;
	for (x = 0; x < 32; x++)
		for (y = 0; y < 32; y++)
		{
			sparse_chunk_2_t *c_2 = (*c_3)[x][y];
			if (c_2)
			{
	int x, y;
	for (x = 0; x < 4; x++)
		for (y = 0; y < 4; y++)
		{
			sparse_chunk_1_t *c_1 = (*c_2)[x][y];
			if (c_1)
			{

	afree(c_1);
			}
		}
	afree(c_2);
			}
		}
	afree(c_3);
}

/* section: lookup values */

static inline unsigned char lookup_sparse(sparse_arr c_3, int x, int y)
{
	sparse_chunk_2_t *c_2 = (*c_3)[(x>>5)&31][(y>>5)&31];
	if (c_2)
	{
			sparse_chunk_1_t *c_1 = (*c_2)[(x>>3)&3][(y>>3)&3];
	if (c_1)
	{
			return (*c_1)[(x>>0)&7][(y>>0)&7];
	}
	else
		return 0x00;	

	}
	else
		return 0x00;	

}

/* section: inserting values */
static inline void insert_sparse(sparse_arr c_3, int x, int y, unsigned char datum)
{
	/* variable declarations*/
	sparse_chunk_1_t *c_1;
	sparse_chunk_2_t *c_2;

	/* body */
	c_2 = (*c_3)[(x>>5)&31][(y>>5)&31];
	if (c_2 == NULL)
	{
		/* allocate and initialize */
		int i, j;
		c_2 = amalloc(sizeof(sparse_chunk_2_t));
		for (i = 0; i < 4; i++)
			for (j = 0; j < 4; j++)
				(*c_2)[i][j] = NULL;

		/* place back in higher level */
		(*c_3)[(x>>5)&31][(y>>5)&31] = c_2;
	}

	/* next level */
	c_1 = (*c_2)[(x>>3)&3][(y>>3)&3];
	if (c_1 == NULL)
	{
		/* allocate and initialize */
		int i, j;
		c_1 = amalloc(sizeof(sparse_chunk_1_t));
		for (i = 0; i < 8; i++)
			for (j = 0; j < 8; j++)
				(*c_1)[i][j] = 0x00;

		/* place back in higher level */
		(*c_2)[(x>>3)&3][(y>>3)&3] = c_1;
	}

	/* next level */
	(*c_1)[(x>>0)&7][(y>>0)&7] = datum;
}

/* section: clean up and minimize the memory usage */

static void cleanup_sparse(sparse_arr c_3)
{
	int x, y, chunkempty;
	for (x = 0; x < 32; x++)
		for (y = 0; y < 32; y++)
		{
			int thisfull = 0, needtofree = 0;
			sparse_chunk_2_t *c_2 = (*c_3)[x][y];
			if (c_2)
			{
				int x, y, chunkempty = 1;
				thisfull = 1;
	for (x = 0; x < 4; x++)
		for (y = 0; y < 4; y++)
		{
			int thisfull = 0, needtofree = 0;
			sparse_chunk_1_t *c_1 = (*c_2)[x][y];
			if (c_1)
			{
				int x, y, chunkempty = 1;
				thisfull = 1;
	for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++)
			if ((*c_1)[x][y] != 0x00)
				chunkempty = 0;
				if (chunkempty)
					needtofree = 1;
			}
			if (needtofree)
			{
				afree(c_1);
				(*c_2)[x][y] = NULL;
				thisfull = 0;
			}
			if (thisfull)
				chunkempty = 0;
		}
				if (chunkempty)
					needtofree = 1;
			}
			if (needtofree)
			{
				afree(c_2);
				(*c_3)[x][y] = NULL;
				thisfull = 0;
			}
			if (thisfull)
				chunkempty = 0;
		}
}

/* section: figure out how much memory this is using */

static void sparse_allocated(sparse_arr c_3, int *bytesp, int *blocksp)
{
	int x, y, bytes = *bytesp, blocks = *blocksp;
	bytes += sizeof(sparse_chunk_3_t);
	blocks++;
	for (x = 0; x < 32; x++)
		for (y = 0; y < 32; y++)
		{
			sparse_chunk_2_t *c_2 = (*c_3)[x][y];
			if (c_2)
			{
				int x, y;
				bytes += sizeof(sparse_chunk_2_t);
				blocks++;
	for (x = 0; x < 4; x++)
		for (y = 0; y < 4; y++)
		{
			sparse_chunk_1_t *c_1 = (*c_2)[x][y];
			if (c_1)
			{
				int x, y;
				bytes += sizeof(sparse_chunk_1_t);
				blocks++;
(void)(x+y); /* shut up compiler warning */
			}
		}
			}
		}
	*bytesp = bytes;
	*blocksp = blocks;
}

/* done */
