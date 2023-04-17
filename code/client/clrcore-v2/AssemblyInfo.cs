using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.Versioning;

[assembly: TargetFramework(".NETFramework,Version=v4.5", FrameworkDisplayName = ".NET Framework 4.5")]
[assembly: AssemblyTitle("CitizenFX.Core V2")]

#if IS_FXSERVER
[assembly: AssemblyDescription("CitizenFX.Core for FXServer")]
#elif GTA_FIVE
[assembly: AssemblyDescription("CitizenFX.Core for FiveM")]
[assembly: InternalsVisibleTo("CitizenFX.FiveM")]
[assembly: InternalsVisibleTo("CitizenFX.FiveM.NativeImpl")]
#elif IS_RDR3
[assembly: AssemblyDescription("CitizenFX.Core for RedM")]
[assembly: InternalsVisibleTo("CitizenFX.RedM")]
[assembly: InternalsVisibleTo("CitizenFX.RedM.NativeImpl")]
#elif GTA_NY
[assembly: AssemblyDescription("CitizenFX.Core for LibertyM")]
[assembly: InternalsVisibleTo("CitizenFX.LibertyM")]
[assembly: InternalsVisibleTo("CitizenFX.LibertyM.NativeImpl")]
#endif

[assembly: AssemblyCompany("The CitizenFX Collective")]
[assembly: AssemblyCopyright("Copyright CitizenFX and contributors, licensed under the FiveM Platform Agreement")]

[assembly: AssemblyVersion("2.0.0.0")]
[assembly: AssemblyFileVersion("2.0.0.0")]
