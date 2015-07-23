#include <stdio.h>
#define BUFFER_LEN 32000
#define MAX_ELEM_COUNT 50

int main(int argc,char **argv)
{
	int i,j,k;
	unsigned short offset[MAX_ELEM_COUNT]; // Dialog elements offsets
	int offsetsNum; // Total number of elements
	char *inputFile;
	char *outputFile;
	char rows, columns; // Dimensions of text window
	unsigned char buffer[BUFFER_LEN]; // Input buffer

	int fileSize; // Calculated input file size
	FILE *in, *out;

	printf("Captain Comic II : Fractured Reality\r\n");
	printf("Text dialogue conversion utility\r\n");
	printf("It decodes a binary data to sensible ASCII-text\r\n");
	printf("(c) 2015 Pavel Keyno and Ceidwad from Shikadi.net\r\n\r\n");
	if(argc<3)
	{
		printf("Text data of the game located in frNNN.3 files\r\n");
		printf("Example: %s fr000.3 fr000.txt", argv[0]);
	}
	else
	{
		inputFile = argv[1];
		outputFile = argv[2];
		in = fopen(inputFile, "rb");
		if(!in)
		{
			fprintf(stderr, "Couldn't open input file %s\r\n", inputFile);
			return 1;
		}
		else
		{
			i=0;
			while (!feof(in) && i<BUFFER_LEN)
			{
				buffer[i]=fgetc(in);
				buffer[i]=buffer[i]^0x25; // Decrypting
				i++;
			}
			fileSize = i;
			if(i>=BUFFER_LEN)
			{
				fprintf(stderr, "The input file is too big for processing.\r\nProbably, it's not a Captain Comic II story file.\r\n");
				return 1;
			}
		}
		fclose(in);

		// Reading first offset
		offset[0] = buffer[1];
		offset[0] = offset[0] << 8;
		offset[0] = offset[0] + buffer[0];

		offsetsNum = offset[0]/2; // Counting total offsets
		if(offsetsNum>MAX_ELEM_COUNT || offset[0]>fileSize)
		{
			fprintf(stderr, "Too many text dialogs in the file or first offset exceeds input file size. It seems thats not a Captain Comic II dialog file.\r\n");
			return 1;
		}
		for(i=1;i<offsetsNum;i++) // Reading other offsets
		{
			offset[i] = buffer[i*2+1];
			offset[i] = offset[i] << 8;
			offset[i] = offset[i] + buffer[i*2];
			if(offset[i]>fileSize)
			{
				fprintf(stderr, "The offset %d exceeds input file size\r\n", i);
				return 1;
			}
		}
		printf("The first offset is %d\r\n", offset[0]);

		out = fopen(outputFile, "wt");
		if(!out)
		{
			fprintf(stderr, "Can't create output file %s.\r\nProbably, disk full or write-protected.\r\n", outputFile);
			return 1;
		}
		else
		{
			for(i=0;i<offsetsNum;i++) // Starts reading the text elements
			{
				columns = buffer[offset[i]]-2; // Calculating rows and columns num
				rows = buffer[offset[i]+1]-2;  // Two chars are used for the frame dialog, so substract them

				k=0;
				for(j=0;j<rows*columns;j++)
				{
					fputc(buffer[offset[i]+2+j], out);
					k++;
					if(k==columns)
					{
						fputc('\n', out);
						k=0;
					}
				}
				if(i<offsetsNum-1) // Printing delimiter
				{
					fputc('-', out); fputc('-', out); fputc('\n', out);
				}
			}
			fclose(out);
			printf("Finished! Extracted %d dialog(s).\r\n", offsetsNum);
		}
	}
	return 0;
}
