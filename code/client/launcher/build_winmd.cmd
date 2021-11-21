@echo off
midl /winrt /metadata_dir "%WindowsSdkDir%References\10.0.22000.0\windows.foundation.foundationcontract\4.0.0.0" /h "nul" /nomidl /reference "%WindowsSdkDir%References\10.0.22000.0\Windows.Foundation.FoundationContract\4.0.0.0\Windows.Foundation.FoundationContract.winmd" /reference "%WindowsSdkDir%References\10.0.22000.0\Windows.Foundation.UniversalApiContract\13.0.0.0\Windows.Foundation.UniversalApiContract.winmd" BackdropBrush.idl

cppwinrt -in BackdropBrush.winmd -c -r sdk -o .

del module.g.cpp
