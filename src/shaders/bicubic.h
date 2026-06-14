//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//
//
//                    BICUBIC IMAGE SCALING
//
//
//------------------------------------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//                                                      CONSTANT SETUP
//==============================================================================================================================
// Call to setup required constant values (works on CPU or GPU).
FFX_STATIC void BicubicCon(
FFX_PARAMETER_OUT FfxFloat32x4 con0,
FFX_PARAMETER_OUT FfxFloat32x4 con1,
// Configurable parameters
FfxFloat32 B,
FfxFloat32 C,
// This the rendered image resolution
FfxFloat32 inputRenderedSizeX,
FfxFloat32 inputRenderedSizeY,
// This is the resolution of the resource containing the input image (useful for dynamic resolution)
FfxFloat32 inputCurrentSizeX,
FfxFloat32 inputCurrentSizeY,
// This is the window width / height
FfxFloat32 outputTargetSizeX,
FfxFloat32 outputTargetSizeY)
{
 // Input/Output size information
 con0[0]=inputRenderedSizeX;
 con0[1]=inputRenderedSizeY;
 con0[2]=inputCurrentSizeX;
 con0[3]=inputCurrentSizeY;

 // Viewport pixel position to normalized image space.
 con1[0]=outputTargetSizeX;
 con1[1]=outputTargetSizeY;
 con1[2]=B;
 con1[3]=C;
}
