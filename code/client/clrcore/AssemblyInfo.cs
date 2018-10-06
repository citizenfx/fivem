using System.Reflection;
using System.Runtime.Versioning;

[assembly: TargetFramework(".NETFramework,Version=v4.5", FrameworkDisplayName = ".NET Framework 4.5")]
[assembly: AssemblyTitle("CitizenFX.Core")]

#if GTA_FIVE
[assembly: AssemblyDescription("CitizenFX.Core for GTA_FIVE")]
#elif IS_FXSERVER
[assembly: AssemblyDescription("CitizenFX.Core for FXServer")]
#endif

[assembly: AssemblyCompany("The CitizenFX Collective")]
[assembly: AssemblyCopyright("Copyright CitizenFX and contributors, licensed under the FiveM Platform Agreement")]

