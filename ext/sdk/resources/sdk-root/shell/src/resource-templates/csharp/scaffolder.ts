import { inject, injectable } from 'inversify';
import { v4 as uuid } from 'uuid';
import * as cp from 'child_process';
import { FsService } from 'backend/fs/fs-service';
import { concurrently } from 'utils/concurrently';
import { ResourceTemplateScaffolder, ResourceTemplateScaffolderArgs } from "../types";
import { TaskReporterService } from 'backend/task/task-reporter-service';

const resourceNameRegexp = /([a-zA-Z0-9]+)/g;

function ucfirst(str: string): string {
  return str[0].toUpperCase() + str.substr(1);
}

function resourceNameToNamespace(resourceName: string): string {
  const matches = resourceName.match(resourceNameRegexp) || [];

  return matches.map(ucfirst).join('');
}

@injectable()
export default class CsharpScaffolder implements ResourceTemplateScaffolder {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  async scaffold({ request, manifest, resourcePath }: ResourceTemplateScaffolderArgs) {
    this.taskReporterService.wrap(`Scaffolding ${request.assetName} resource from C# template`, async () => {
      const resourceName = request.assetName;

      manifest.files.push('Client/bin/Release/**/publish/*.dll');
      manifest.clientScripts.push('Client/bin/Release/**/publish/*.net.dll');
      manifest.serverScripts.push('Server/bin/Release/**/publish/*.net.dll');

      manifest.fxdkWatchCommands.push(['dotnet', [
        'watch', '--project', `Client/${resourceName}.Client.csproj`,
        'publish', '--configuration', 'Release',
      ]]);
      manifest.fxdkWatchCommands.push(['dotnet', [
        'watch', '--project', `Server/${resourceName}.Server.csproj`,
        'publish', '--configuration', 'Release',
      ]]);

      const promises: Promise<any>[] = [];
      const relativePath = (to: string) => this.fsService.joinPath(resourcePath, to);

      // Create solution
      promises.push(this.fsService.writeFile(relativePath(resourceName + '.sln'), getSolutionFileContent(resourceName)));

      // Create shared
      promises.push(
        this.fsService.mkdirp(relativePath('Shared'))
          .then(() => this.fsService.writeFile(relativePath('Shared/Dummy.cs'), '')),
      );

      // Create client project
      promises.push(
        this.fsService.mkdirp(relativePath('Client'))
          .then(() => concurrently(
            this.fsService.writeFile(relativePath(`Client/${resourceName}.Client.csproj`), getClientProjectFileContent()),
            this.fsService.writeFile(relativePath(`Client/ClientMain.cs`), getClientScriptFileContent(resourceName)),
          )),
      );

      // Create server project
      promises.push(
        this.fsService.mkdirp(relativePath('Server'))
          .then(() => concurrently(
            this.fsService.writeFile(relativePath(`Server/${resourceName}.Server.csproj`), getServerProjectFileContent()),
            this.fsService.writeFile(relativePath(`Server/ServerMain.cs`), getServerScriptFileContent(resourceName)),
          )),
      );

      await Promise.all(promises);

      await this.installSdk();
      await this.restoreProjects(resourcePath);
    });
  }

  private async installSdk() {
    return new Promise<void>((resolve, reject) => {
      cp.exec('dotnet new -i CitizenFX.Templates', { windowsHide: true }, (err) => {
        if (err) {
          return reject(err);
        }

        return resolve();
      })
    });
  }

  private async restoreProjects(resourcePath: string) {
    return new Promise<void>((resolve, reject) => {
      cp.exec('dotnet restore', { cwd: resourcePath, windowsHide: true }, (err) => {
        if (err) {
          return reject(err);
        }

        return resolve();
      })
    });
  }
}

function getSolutionFileContent(resourceName: string): string {
  const solutionUuid = uuid().toUpperCase();
  const clientUuid = uuid().toUpperCase();
  const serverUuid = uuid().toUpperCase();

  return `
Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio 15
VisualStudioVersion = 15.0.26124.0
MinimumVisualStudioVersion = 15.0.26124.0
Project("{${solutionUuid}}") = "${resourceName}.Client", "Client\\${resourceName}.Client.csproj", "{${clientUuid}}"
EndProject
Project("{${solutionUuid}}") = "${resourceName}.Server", "Server\\${resourceName}.Server.csproj", "{${serverUuid}}"
EndProject
Global
  GlobalSection(SolutionConfigurationPlatforms) = preSolution
    Debug|Any CPU = Debug|Any CPU
    Release|Any CPU = Release|Any CPU
  EndGlobalSection
  GlobalSection(SolutionProperties) = preSolution
    HideSolutionNode = FALSE
  EndGlobalSection
  GlobalSection(ProjectConfigurationPlatforms) = postSolution
    {${clientUuid}}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
    {${clientUuid}}.Debug|Any CPU.Build.0 = Debug|Any CPU
    {${clientUuid}}.Release|Any CPU.ActiveCfg = Release|Any CPU
    {${clientUuid}}.Release|Any CPU.Build.0 = Release|Any CPU
    {${serverUuid}}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
    {${serverUuid}}.Debug|Any CPU.Build.0 = Debug|Any CPU
    {${serverUuid}}.Release|Any CPU.ActiveCfg = Release|Any CPU
    {${serverUuid}}.Release|Any CPU.Build.0 = Release|Any CPU
  EndGlobalSection
EndGlobal
`.trimStart();
}

function getClientProjectFileContent(): string {
  return `
<Project Sdk="CitizenFX.Sdk.Client/0.2.2">
  <ItemGroup>
      <Compile Include="../Shared/**/*.cs" />
  </ItemGroup>
</Project>
`.trimStart();
}

function getServerProjectFileContent(): string {
  return `
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>netstandard2.0</TargetFramework>
    <DebugType>portable</DebugType>
    <TargetName>$(AssemblyName).net</TargetName>
    <DefineConstants>SERVER</DefineConstants>
  </PropertyGroup>

  <ItemGroup>
    <PackageReference Include="CitizenFX.Core.Server" Version="1.0.*" />

    <Compile Include="../Shared/**/*.cs" />
  </ItemGroup>
</Project>
`.trimStart();
}

function getClientScriptFileContent(resourceName: string): string {
  const namespaceName = resourceNameToNamespace(resourceName);

  return `
using System;
using System.Threading.Tasks;
using CitizenFX.Core;
using static CitizenFX.Core.Native.API;

namespace ${namespaceName}.Client
{
    public class ClientMain : BaseScript
    {
        public ClientMain()
        {
            Debug.WriteLine("Hi from ${namespaceName}.Client!");
        }

        [Tick]
        public Task OnTick()
        {
            DrawRect(0.5f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 150);

            return Task.FromResult(0);
        }
    }
}
`.trimStart();
}

function getServerScriptFileContent(resourceName: string): string {
  const namespaceName = resourceNameToNamespace(resourceName);

  return `
using System;
using System.Threading.Tasks;
using CitizenFX.Core;

namespace ${namespaceName}.Server
{
    public class ServerMain : BaseScript
    {
        public ServerMain()
        {
            Debug.WriteLine("Hi from ${namespaceName}.Server!");
        }

        [Command("hello_server")]
        public void HelloServer()
        {
            Debug.WriteLine("Sure, hello.");
        }
    }
}
`.trimStart();
}
