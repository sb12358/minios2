#include <windows.h>
#include <stdio.h>

BYTE* image;
char filename[256];
char filepath[256];

IMAGE_DOS_HEADER dos_header;
DWORD signature;

DWORD fileheaderoffset;
IMAGE_FILE_HEADER file_header;

DWORD optionalheaderoffset;
IMAGE_OPTIONAL_HEADER optional_header;

DWORD section_number;
DWORD sectiontableoffset;
IMAGE_SECTION_HEADER section[16];

BOOL fLoad;

DWORD LoadImage(char*file)
{
	FILE *fp=fopen(file,"rb");										//Read Dos Header

	if(fp==0)
		return 0;

	fread(&dos_header,sizeof(IMAGE_DOS_HEADER),1,fp);

	fseek(fp,dos_header.e_lfanew,SEEK_SET);						//ReadSignature
	DWORD sig;
	fread(&sig,4,1,fp);

	if(LOWORD(sig)==IMAGE_OS2_SIGNATURE || LOWORD(sig)==IMAGE_OS2_SIGNATURE_LE) 
	{
		fseek(fp,dos_header.e_lfanew+2,SEEK_SET);					//Locate at FileHeader
		signature=LOWORD(sig);
		fileheaderoffset=dos_header.e_lfanew+2;
	}else if(sig==IMAGE_NT_SIGNATURE)
	{
		fseek(fp,dos_header.e_lfanew+4,SEEK_SET);					//Locate at FileHeader
		signature=sig;
		fileheaderoffset=dos_header.e_lfanew+4;
	}else
	{
		fclose(fp);
		return S_FALSE;
	}

	fread(&file_header, sizeof(IMAGE_FILE_HEADER),1,fp);			//Read File Header
	
	optionalheaderoffset=fileheaderoffset+sizeof(IMAGE_FILE_HEADER);	//Read Optional Header
	fread(&optional_header,file_header.SizeOfOptionalHeader,1,fp);

	section_number=file_header.NumberOfSections;				//Read Section Table
	sectiontableoffset=optionalheaderoffset+file_header.SizeOfOptionalHeader;
	if(section_number>16)
		return S_FALSE;
	fread(section, sizeof(IMAGE_SECTION_HEADER),section_number,fp);

	image=(BYTE*)malloc(optional_header.SizeOfImage);
	fseek(fp,0,SEEK_SET);
	fread(image, 0x1000, 1, fp);								// Read Headers
	for(DWORD i=0;i<section_number;i++)
	{
		fseek(fp,section[i].PointerToRawData,SEEK_SET);			// Locate at a section
		fread(image+section[i].VirtualAddress, section[i].SizeOfRawData, 1, fp);
	}

	fclose(fp);
	fLoad=TRUE;
	return TRUE;
}

void Relocate(DWORD NewBase)
{
	DWORD Delta = (DWORD)NewBase - optional_header.ImageBase;

	PIMAGE_BASE_RELOCATION pLoc = (PIMAGE_BASE_RELOCATION)((unsigned long)image
	+ optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	while((pLoc->VirtualAddress + pLoc->SizeOfBlock) != 0)
	{
		WORD *pLocData = (WORD *)((int)pLoc + sizeof(IMAGE_BASE_RELOCATION));
		int NumberOfReloc = (pLoc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION))/sizeof(WORD);
		for( int i=0 ; i < NumberOfReloc; i++)
		{
			if( (DWORD)(pLocData[i] & 0xF000) == 0x00003000)
			{
				DWORD * pAddress = (DWORD *)((unsigned long)image + pLoc->VirtualAddress + (pLocData[i] & 0x0FFF));
				*pAddress += Delta;
			}
		}
		pLoc = (PIMAGE_BASE_RELOCATION)((DWORD)pLoc + pLoc->SizeOfBlock);
	}
}

void makedllbin(char *dllfile, char *bootfile, char *binfile, DWORD base)
{
	char bootsector[512];
	FILE *boot=fopen(bootfile, "rb");
	fread(bootsector, 512, 1, boot);
	fclose(boot);	
	printf("Relocate %s at 0x%0x\n", dllfile, base);
	LoadImage(dllfile);
	Relocate(base);

	*((int *)&bootsector[504])=1;
	*((short *)&bootsector[508])=(short)(optional_header.SizeOfImage+511)/512;

	DWORD r;
	HANDLE fp=CreateFile(binfile,GENERIC_READ|GENERIC_WRITE,0, NULL, OPEN_ALWAYS, 0, NULL);
	WriteFile(fp, bootsector, 512, &r, NULL);
	WriteFile(fp, image, optional_header.SizeOfImage, &r, NULL);
	CloseHandle(fp);
	printf("The Bootable Image at %s\n", binfile);

	return;
}

void copybin(char *dllfile, char *bootfile, char *binfile)
{
	char *bootsector=(char*)malloc(512);
	FILE *fp= fopen(bootfile, "rb");
	fread(bootsector, 512, 1, fp);
	fclose(fp);

	fp=fopen(dllfile, "rb");
	fseek(fp, 0, SEEK_END);
	int imagelen=ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *image=(char*)malloc(imagelen);
	fread(image, imagelen, 1, fp);
	fclose(fp);

	*((int *)&bootsector[504])=1;
	*((short *)&bootsector[508])=(imagelen+511)/512;

	DWORD r;
	HANDLE fpbin=CreateFile(binfile,GENERIC_READ|GENERIC_WRITE,0, NULL, OPEN_ALWAYS, 0, NULL);
	WriteFile(fpbin, bootsector, 512, &r, NULL);
	WriteFile(fpbin, image, imagelen, &r, NULL);
	CloseHandle(fpbin);
	printf("The Bootable Image at %s\n", binfile);
}


int main( int argc, char *argv[ ])
{
	if(argc<4)
	{
		//copybin("minios.dll", "bootsector", "minios.vhd");
		makedllbin("minios.dll", "bootsector", "MS-DOS-flat.vmdk", 0x400000);
	}
	else
	{
		//copybin(argv[1], argv[2], argv[3]);
		makedllbin(argv[1], argv[2], argv[3], 0x400000);
	}
	return 0;
}