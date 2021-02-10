import * as React from 'react';
import URI from '@theia/core/lib/common/uri';
import { injectable, inject, postConstruct } from 'inversify';
import { CommandRegistry, isOSX, environment, Path } from '@theia/core/lib/common';
import { WorkspaceCommands, WorkspaceService } from '@theia/workspace/lib/browser';
import { KeymapsCommands } from '@theia/keymaps/lib/browser';
import { CommonCommands, LabelProvider } from '@theia/core/lib/browser';
import { ApplicationInfo, ApplicationServer } from '@theia/core/lib/common/application-protocol';
import { FrontendApplicationConfigProvider } from '@theia/core/lib/browser/frontend-application-config-provider';
import { EnvVariablesServer } from '@theia/core/lib/common/env-variables';
import { GettingStartedWidget } from '@theia/getting-started/lib/browser/getting-started-widget';

/**
 * Default implementation of the `GettingStartedWidget`.
 * The widget is displayed when there are currently no workspaces present.
 * Some of the features displayed include:
 * - `open` commands.
 * - `recently used workspaces`.
 * - `settings` commands.
 * - `help` commands.
 * - helpful links.
 */
@injectable()
export class FxdkFAQWidget extends GettingStartedWidget {

  /**
   * The `ApplicationInfo` for the application if available.
   * Used in order to obtain the version number of the application.
   */
  protected applicationInfo: ApplicationInfo | undefined;
  /**
   * The application name which is used for display purposes.
   */
  protected applicationName = FrontendApplicationConfigProvider.get().applicationName;

  protected home: string | undefined;

  /**
   * The recently used workspaces limit.
   * Used in order to limit the number of recently used workspaces to display.
   */
  protected recentLimit = 5;
  /**
   * The list of recently used workspaces.
   */
  protected recentWorkspaces: string[] = [];

  /**
   * Collection of useful links to display for end users.
   */
  protected readonly documentationUrl = 'https://www.theia-ide.org/docs/';
  protected readonly extensionUrl = 'https://www.theia-ide.org/docs/authoring_extensions';
  protected readonly pluginUrl = 'https://www.theia-ide.org/docs/authoring_plugins';

  @inject(ApplicationServer)
  protected readonly appServer: ApplicationServer;

  @inject(CommandRegistry)
  protected readonly commandRegistry: CommandRegistry;

  @inject(EnvVariablesServer)
  protected readonly environments: EnvVariablesServer;

  @inject(LabelProvider)
  protected readonly labelProvider: LabelProvider;

  @inject(WorkspaceService)
  protected readonly workspaceService: WorkspaceService;

  @postConstruct()
  protected async init(): Promise<void> {
    this.id = GettingStartedWidget.ID;
    this.title.label = 'FAQ';
    this.title.caption = 'FAQ';
    this.title.closable = true;
    this.title.iconClass = 'fa fa-question';

    this.applicationInfo = await this.appServer.getApplicationInfo();
    this.recentWorkspaces = await this.workspaceService.recentWorkspaces();
    this.home = new URI(await this.environments.getHomeDirUri()).path.toString();
    this.update();
  }

  /**
   * Render the content of the widget.
   */
  protected render(): React.ReactNode {
    return (
      <div className='gs-container'>
        <div className='gs-header'>
          <h1>FxDK<span className='gs-sub-header'> F.A.Q.</span></h1>
        </div>

        <hr className='gs-hr' />

        <div className='fxdk-faq-container'>

          <article>
            <header>
              What is FxDK in a nutshell?
            </header>
            <main>
              FxDK is an <abbr title="Integrated Development Environment">IDE</abbr> for, currently, FiveM.
              <br/>
              It features embedded game view (FxDK -&gt; Toggle Game View), integrated FXServer,
              <br/>
              both server and client consoles as widget (FxDK -&gt; Toggle Client/Server Console), extensive JS/TS and C# support and a lot more.
            </main>
          </article>

          <article>
            <header>
              No Lua support?
            </header>
            <main>
              Unfortunately, all existing vscode Lua language server extensions are not ready for production use.
              <br/>
              For we don't have a manpower to fix existing products or create our own from the scratch.
            </main>
          </article>

          <article>
            <header>
              How do I limit what files get deployed upon project build?
            </header>
            <main>
              By default FxDK will include all files contained in resource root,
              <br/>
              add <kbd>.fxdkignore</kbd> file to your resource root to limit what files get deployed.
              <br/>
              This file's syntax pretty much follows such of <kbd>.gitignore</kbd>.
            </main>
          </article>

          <article>
            <header>
              How do I automate building of resource?
            </header>
            <main>
              If your resource requires building FxDK have support for two building options: watch commands and build commands.
              <br/>
              Watch commands are running if resource is enabled and "Restart on change" is enabled.
              <br/>
              Build commands are for project build stage.
            </main>
          </article>

          <article>
            <header>
              How do I specify watch/build commands?
            </header>
            <main>
              You specify these in you <kbd>fxmanifest.lua</kbd>:
              <br/>
              <kbd>fxdk_watch_command 'yarn' {'{'}'watch', '--mode=development'{'}'}</kbd> - yarn is a command, next part is command args.
              <br/>
              <kbd>fxdk_build_command 'yarn' {'{'}'build'{'}'}</kbd> - same as watch command.
              <br/>
              Do note that you can specify multiple commands of same type, so you can have multiple watch/build commands even with the same command part.
            </main>
          </article>

        </div>
      </div>
    );
  }

  /**
   * Render the widget header.
   * Renders the title `{applicationName} Getting Started`.
   */
  protected renderHeader(): React.ReactNode {
    return <div className='gs-header'>
      <h1>FxDK<span className='gs-sub-header'> Getting Started</span></h1>
    </div>;
  }

  /**
   * Render the `open` section.
   * Displays a collection of `open` commands.
   */
  protected renderOpen(): React.ReactNode {
    const requireSingleOpen = isOSX || !environment.electron.is();
    const open = requireSingleOpen && <div className='gs-action-container'><a href='#' onClick={this.doOpen}>Open</a></div>;
    const openFile = !requireSingleOpen && <div className='gs-action-container'><a href='#' onClick={this.doOpenFile}>Open File</a></div>;
    const openFolder = !requireSingleOpen && <div className='gs-action-container'><a href='#' onClick={this.doOpenFolder}>Open Folder</a></div>;
    const openWorkspace = <a href='#' onClick={this.doOpenWorkspace}>Open Workspace</a>;
    return <div className='gs-section'>
      <h3 className='gs-section-header'><i className='fa fa-folder-open'></i>Open</h3>
      {open}
      {openFile}
      {openFolder}
      {openWorkspace}
    </div>;
  }

  /**
   * Render the recently used workspaces section.
   */
  protected renderRecentWorkspaces(): React.ReactNode {
    const items = this.recentWorkspaces;
    const paths = this.buildPaths(items);
    const content = paths.slice(0, this.recentLimit).map((item, index) =>
      <div className='gs-action-container' key={index}>
        <a href='#' onClick={a => this.open(new URI(items[index]))}>{new URI(items[index]).path.base}</a>
        <span className='gs-action-details'>
          {item}
        </span>
      </div>
    );
    // If the recently used workspaces list exceeds the limit, display `More...` which triggers the recently used workspaces quick-open menu upon selection.
    const more = paths.length > this.recentLimit && <div className='gs-action-container'><a href='#' onClick={this.doOpenRecentWorkspace}>More...</a></div>;
    return <div className='gs-section'>
      <h3 className='gs-section-header'>
        <i className='fa fa-clock-o'></i>Recent Workspaces
            </h3>
      {items.length > 0 ? content : <p className='gs-no-recent'>No Recent Workspaces</p>}
      {more}
    </div>;
  }

  /**
   * Render the settings section.
   * Generally used to display useful links.
   */
  protected renderSettings(): React.ReactNode {
    return <div className='gs-section'>
      <h3 className='gs-section-header'>
        <i className='fa fa-cog'></i>
                Settings
            </h3>
      <div className='gs-action-container'>
        <a href='#' onClick={this.doOpenPreferences}>Open Preferences</a>
      </div>
      <div className='gs-action-container'>
        <a href='#' onClick={this.doOpenKeyboardShortcuts}>Open Keyboard Shortcuts</a>
      </div>
    </div>;
  }

  /**
   * Render the help section.
   */
  protected renderHelp(): React.ReactNode {
    return <div className='gs-section'>
      <h3 className='gs-section-header'>
        <i className='fa fa-question-circle'></i>
                Help
            </h3>
      <div className='gs-action-container'>
        <a href={this.documentationUrl} target='_blank'>Documentation</a>
      </div>
      <div className='gs-action-container'>
        <a href={this.extensionUrl} target='_blank'>Building a New Extension</a>
      </div>
      <div className='gs-action-container'>
        <a href={this.pluginUrl} target='_blank'>Building a New Plugin</a>
      </div>
    </div>;
  }

  /**
   * Render the version section.
   */
  protected renderVersion(): React.ReactNode {
    return <div className='gs-section'>
      <div className='gs-action-container'>
        <p className='gs-sub-header' >
          {this.applicationInfo ? 'Version ' + this.applicationInfo.version : ''}
        </p>
      </div>
    </div>;
  }

  /**
   * Build the list of workspace paths.
   * @param workspaces {string[]} the list of workspaces.
   * @returns {string[]} the list of workspace paths.
   */
  protected buildPaths(workspaces: string[]): string[] {
    const paths: string[] = [];
    workspaces.forEach(workspace => {
      const uri = new URI(workspace);
      const pathLabel = this.labelProvider.getLongName(uri);
      const path = this.home ? Path.tildify(pathLabel, this.home) : pathLabel;
      paths.push(path);
    });
    return paths;
  }

  /**
   * Trigger the open command.
   */
  protected doOpen = () => this.commandRegistry.executeCommand(WorkspaceCommands.OPEN.id);
  /**
   * Trigger the open file command.
   */
  protected doOpenFile = () => this.commandRegistry.executeCommand(WorkspaceCommands.OPEN_FILE.id);
  /**
   * Trigger the open folder command.
   */
  protected doOpenFolder = () => this.commandRegistry.executeCommand(WorkspaceCommands.OPEN_FOLDER.id);
  /**
   * Trigger the open workspace command.
   */
  protected doOpenWorkspace = () => this.commandRegistry.executeCommand(WorkspaceCommands.OPEN_WORKSPACE.id);
  /**
   * Trigger the open recent workspace command.
   */
  protected doOpenRecentWorkspace = () => this.commandRegistry.executeCommand(WorkspaceCommands.OPEN_RECENT_WORKSPACE.id);
  /**
   * Trigger the open preferences command.
   * Used to open the preferences widget.
   */
  protected doOpenPreferences = () => this.commandRegistry.executeCommand(CommonCommands.OPEN_PREFERENCES.id);
  /**
   * Trigger the open keyboard shortcuts command.
   * Used to open the keyboard shortcuts widget.
   */
  protected doOpenKeyboardShortcuts = () => this.commandRegistry.executeCommand(KeymapsCommands.OPEN_KEYMAPS.id);
  /**
   * Open a workspace given its uri.
   * @param uri {URI} the workspace uri.
   */
  protected open = (uri: URI) => this.workspaceService.open(uri);
}
