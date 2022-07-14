#include <application.h>


int Marble_Atlas_Create(int iType, void **ptrpAtlas) { MARBLE_ERRNO
	switch (iType) {		
		default: iErrorCode = Marble_ErrorCode_AtlasType;
	}

	return iErrorCode;
}

void Marble_Atlas_Destroy(void **ptrpAtlas) {
	if (ptrpAtlas && *ptrpAtlas) {
		switch (((Marble_Atlas *)*ptrpAtlas)->iAtlasType) {

		}

		free(*ptrpAtlas);
		*ptrpAtlas = NULL;
	}
}


