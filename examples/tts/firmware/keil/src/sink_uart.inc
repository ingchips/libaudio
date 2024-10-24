extern void save_bytes(const void *p, int size);

int do_save_pcm_samples(const int16_t *pcm_samples, int number, int acc_number)
{
    save_bytes(pcm_samples, number * sizeof(pcm_samples[0]));
	return 0;
}

static void init_sink(void)
{
}