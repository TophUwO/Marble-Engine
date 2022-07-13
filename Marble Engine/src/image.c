#include <application.h>


int Marble_Asset_CreateImageAsset(Marble_Asset **ptrpAsset) { MARBLE_ERRNO
	if (iErrorCode = Marble_System_AllocateMemory(ptrpAsset, sizeof(Marble_ImageAsset), FALSE, FALSE))
		return iErrorCode;

	(*ptrpAsset)->iAssetType       = Marble_AssetType_Image;
	(*ptrpAsset)->uqwGlobalAssetId = Marble_AssetManager_RequestAssetId();

	return Marble_ErrorCode_Ok;
}

int Marble_Image_LoadFromFile(Marble_Asset *sImage, TCHAR const *strPath) {
	if (sImage && strPath) {
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
		Marble_ImageAsset *sAcImage = (Marble_ImageAsset *)sImage;
		Marble_IfError(
			D2DWr_DeviceContext_CreateBitmapFromWicBitmap(
				gl_sApplication.sRenderer->sD2DRenderer.sD2DDevContext,
				(IWICBitmapSource *)sWICFmtConverter,
				NULL,
				(ID2D1Bitmap1 **)&sAcImage->sD2DBitmap
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


