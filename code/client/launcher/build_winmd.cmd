@echo off
midl /winrt /metadata_dir "%WindowsSdkDir%References\10.0.18362.0\windows.foundation.foundationcontract\3.0.0.0" /h "nul" /nomidl /reference "%WindowsSdkDir%References\10.0.18362.0\Windows.Foundation.FoundationContract\3.0.0.0\Windows.Foundation.FoundationContract.winmd" /reference "%WindowsSdkDir%References\10.0.18362.0\Windows.Foundation.UniversalApiContract\8.0.0.0\Windows.Foundation.UniversalApiContract.winmd" BackdropBrush.idl

cppwinrt -v -in BackdropBrush.winmd -c -r sdk -o .

cppwinrt -v -in "..\..\..\vendor\win2d\bin\uapx64\debug\MergedWinMD\Microsoft.Graphics.Canvas.winmd" -r sdk -o .

del module.g.cpp