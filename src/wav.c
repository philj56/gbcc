#include "wav.h"

void wav_parse_header(struct wav_header *header, FILE *wav)
{
	fread(header->ChunkID, 1, 4, wav);
	fread(&header->ChunkSize, 4, 1, wav);
	fread(header->Format, 1, 4, wav);
	
	fread(header->Subchunk1ID, 1, 4, wav);
	fread(&header->Subchunk1Size, 4, 1, wav);
	fread(&header->AudioFormat, 2, 1, wav);
	fread(&header->NumChannels, 2, 1, wav);
	fread(&header->SampleRate, 4, 1, wav);
	fread(&header->ByteRate, 4, 1, wav);
	fread(&header->BlockAlign, 2, 1, wav);
	fread(&header->BitsPerSample, 2, 1, wav);

	fread(header->Subchunk2ID, 1, 4, wav);
	fread(&header->Subchunk2Size, 4, 1, wav);
}

void wav_print_header(struct wav_header *header)
{
	//printf(header->ChunkID, 1, 4, wav);
	printf("ChunkSize:\t0x%08X\n", header->ChunkSize);
	//printf("Format:\t0x%08X\n", header->Format);

	//printf("Subchunk1ID:\t0x%08X\n", header->Subchunk1ID);
	printf("Subchunk1Size:\t0x%08X\n", header->Subchunk1Size);
	printf("AudioFormat:\t0x%04X\n", header->AudioFormat);
	printf("NumChannels:\t0x%04X\n", header->NumChannels);
	printf("SampleRate:\t0x%08X\n", header->SampleRate);
	printf("ByteRate:\t0x%08X\n", header->ByteRate);
	printf("BlockAlign:\t0x%04X\n", header->BlockAlign);
	printf("BitsPerSample:\t0x%04X\n", header->BitsPerSample);

	//printf("Subchunk2ID:\t0x%08X\n", header->Subchunk2ID);
	printf("Subchunk2Size:\t0x%08X\n", header->Subchunk2Size);
}
