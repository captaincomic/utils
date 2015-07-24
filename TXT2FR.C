/**
 * @file   txt2fr.c
 * @brief  Captain Comic II dialog converter from ASCII-text to binary
 *
 * Copyright (C) 2015 Pavel Keyno <src@keyno.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#define BUFFER_LEN 32000
#define MAX_ELEM_COUNT 50

// Simple encode function macros
#define fputc_enc(c, f) fputc((c^0x25), (f))

typedef struct RowCol {
	unsigned char columns;
	unsigned char rows;
} rowcol;

int main(int argc,char **argv)
{
	int i,j,k;
	unsigned short offset[MAX_ELEM_COUNT]; // Dialog elements offsets
	unsigned short realOffset[MAX_ELEM_COUNT];
	int offsetsNum; // Total number of elements
	char *inputFile;
	char *outputFile;
	rowcol RaC[MAX_ELEM_COUNT]; // Struct of rows and columns for each dialog
	char tmp[2]; // Temporary array for short int conversion
	unsigned char buffer[BUFFER_LEN]; // Input buffer

	int fileSize; // Calculated input file size
	FILE *in, *out;

	printf("Captain Comic II : Fractured Reality\r\n");
	printf("Text dialogue conversion utility\r\n");
	printf("It encodes a user ASCII-text to binary data\r\n");
	printf("(c) 2015 Pavel Keyno and Ceidwad from Shikadi.net\r\n\r\n");
	if(argc<3)
	{
		printf("Text data of the game located in frNNN.3 files\r\n");
		printf("Each dialog in the text file must be separated by '--' (without quotes) \r\nand by the new line.\r\n");
		printf("Usage: %s fr000.txt fr000.3", argv[0]);
	}
	else
	{
		inputFile = argv[1];
		outputFile = argv[2];
		in = fopen(inputFile, "rt");
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
				i++;
			}
			fileSize = i;
			if(i>=BUFFER_LEN)
			{
				fprintf(stderr, "The input file is too big for processing.\r\nProbably, it has too much dialogs.\r\n");
				return 1;
			}
		}
		fclose(in);

		offset[0]=0;
		offsetsNum=0;
		j=k=0; // Dialogs and string counter
		RaC[0].rows=0;
		RaC[0].columns=0;
		for(i=0;i<fileSize;i++)
		{
			switch(buffer[i])
			{
				case '\n':
					if(RaC[j].rows==0 && k==0) // if string is empty
						offset[j]++;
					else
					{
					  if(RaC[j].columns<k)
						RaC[j].columns=k;
					  RaC[j].rows++;
					}
					k=0;
				break;
				case '-':
					if(buffer[i+1]=='-')
					{
						j++;
						i++;
						k=0;
						RaC[j].rows=0;
						RaC[j].columns=0;
						offset[j]=i+1;
						offsetsNum++;
					}
				break;
				default:
					k++;
				break;
			}
		}
// Need to investigate some test cases with empty strings
//		if(RaC[j].rows==0 && RaC[j].columns==0)
//			offsetsNum--;

		out = fopen(outputFile, "wb");
		if(!out)
		{
			fprintf(stderr, "Can't create output file %s.\r\nProbably, disk full or write-protected.\r\n", outputFile);
			return 1;
		}
		else
		{
			realOffset[0]=offsetsNum*2;
			j=0;
			do // Writing offsets
			{
				(*(unsigned short*)tmp) = realOffset[j];
				fputc_enc(tmp[0], out); // Little endian
				fputc_enc(tmp[1], out);
				printf("Found dialog %d at %d\r\n", j, realOffset[j]);
				j++;
				realOffset[j]=RaC[j-1].columns*RaC[j-1].rows+2+realOffset[j-1];
			}
			while(j<offsetsNum);

			for(j=0;j<offsetsNum;j++) // Starts writing the text elements
			{
				fputc_enc(RaC[j].columns+2, out); // Writing dimensions
				fputc_enc(RaC[j].rows+2, out);    // Two bytes added for frame

				k=0;
				for(i=offset[j];i<fileSize && (buffer[i]!='-' || buffer[i+1]!='-');i++)
				{
					switch(buffer[i])
					{
						case '\n':
							while(RaC[j].columns-k>0) // Filling by spaces
							{
								fputc_enc(0x20, out);
								k++;
							}
							k=0;
						break;
						default:
							fputc_enc(buffer[i], out);
							k++;
						break;
					}
				}
			}
			fputc_enc(0xFF, out); // Adding some shit at the end
			fclose(out);
			printf("Finished! Compiled %d dialog(s).\r\n", offsetsNum);
		}
	}
	return 0;
}
