#include <application.h>


typedef struct Marble_ImageAsset {
	Marble_Asset;

	union {
		ID2D1Bitmap *sD2DBitmap;
	};
} Marble_ImageAsset;


int Marble_Asset_CreateImageAsset(Marble_ImageAsset **ptrpImageAsset) { MARBLE_ERRNO
	if (iErrorCode = Marble_System_AllocateMemory(ptrpImageAsset, sizeof(Marble_ImageAsset), TRUE, FALSE))
		return iErrorCode;

	(*ptrpImageAsset)->iAssetType = Marble_AssetType_Image;

	return Marble_ErrorCode_Ok;
}


int Marble_ImageAsset_LoadFromFile(Marble_ImageAsset *sImageAsset, TCHAR const *strPath) {
	if (sImageAsset && strPath) {
		IWICBitmapDecoder     *sWICDecoder      = NULL;
		IWICBitmapFrameDecode *sWICFrameDecoder = NULL;
		IWICFormatConverter   *sWICFmtConverter = NULL;

		/* Open file */
		Marble_IfError(
			gl_sApplication.sAssets->sWICFactory->lpVtbl->CreateDecoderFromFilename(
				gl_sApplication.sAssets->sWICFactory,
				strPath,
				NULL,
				GENERIC_READ,
				WICDecodeMetadataCacheOnDemand,
				&sWICDecoder
			),
			S_OK,
			return Marble_ErrorCode_CreateWICDecoder
		);

		/* Get first frame from file */
		Marble_IfError(
			sWICDecoder->lpVtbl->GetFrame(
				sWICDecoder, 
				0, 
				&sWICFrameDecoder
			), S_OK, {
				sWICDecoder->lpVtbl->Release(sWICDecoder);

				return Marble_ErrorCode_WICDecoderGetFrame;
			}
		);

		/* Create format converter */
		Marble_IfError(
			gl_sApplication.sAssets->sWICFactory->lpVtbl->CreateFormatConverter(
				gl_sApplication.sAssets->sWICFactory,
				&sWICFmtConverter
			), S_OK, {
				sWICDecoder->lpVtbl->Release(sWICDecoder);
				sWICFrameDecoder->lpVtbl->Release(sWICFrameDecoder);

				return Marble_ErrorCode_WICCreateFormatConv;
			}
		);

		/* Convert bitmap to target format */
		Marble_IfError(
			sWICFmtConverter->lpVtbl->Initialize(
				sWICFmtConverter,
				(IWICBitmapSource *)sWICFrameDecoder,
				&GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.0,
				WICBitmapPaletteTypeCustom
			), S_OK, {
				sWICDecoder->lpVtbl->Release(sWICDecoder);
				sWICFrameDecoder->lpVtbl->Release(sWICFrameDecoder);
				sWICFmtConverter->lpVtbl->Release(sWICFmtConverter);

				return Marble_ErrorCode_WICFormatConversion;
			}
		);

		/* Create Direct2D bitmap */
		Marble_IfError(
			D2DWr_DeviceContext_CreateBitmapFromWicBitmap(
				gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext,
				(IWICBitmapSource *)sWICFmtConverter,
				NULL,
				(ID2D1Bitmap1 **)&sImageAsset->sD2DBitmap
			), S_OK, {
				sWICDecoder->lpVtbl->Release(sWICDecoder);
				sWICFrameDecoder->lpVtbl->Release(sWICFrameDecoder);
				sWICFmtConverter->lpVtbl->Release(sWICFmtConverter);

				return Marble_ErrorCode_CreateD2DBitmapFromIWICBitmap;
			}
		);

		sWICDecoder->lpVtbl->Release(sWICDecoder);
		sWICFrameDecoder->lpVtbl->Release(sWICFrameDecoder);
		sWICFmtConverter->lpVtbl->Release(sWICFmtConverter);

		return Marble_ErrorCode_Ok;
	}

	return Marble_ErrorCode_Parameter;
}

void Marble_ImageAsset_Destroy(Marble_ImageAsset *sImage) {
	if (!sImage)
		return;

	D2DWr_Bitmap_Release(sImage->sD2DBitmap);
}


