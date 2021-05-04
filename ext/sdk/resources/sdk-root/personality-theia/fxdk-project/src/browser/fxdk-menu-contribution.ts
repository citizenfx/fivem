import { injectable, inject } from 'inversify';
import { Command, CommandContribution, CommandHandler, CommandRegistry, MAIN_MENU_BAR, MenuContribution, MenuModelRegistry } from '@theia/core';
import { EditorManager } from '@theia/editor/lib/browser/editor-manager';
import { EDITOR_CONTEXT_MENU } from '@theia/editor/lib/browser/editor-menu';
import { TextEditor } from '@theia/editor/lib/browser/editor';
import { ServerConsoleViewContribution, SERVER_CONSOLE_WIDGET_ICON } from './console/server-console';
import { ClientConsoleViewContribution, CLIENT_CONSOLE_WIDGET_ICON } from './console/client-console';
import { FxdkDataService } from 'fxdk-services/lib/browser/fxdk-data-service';

function formatArrayOfFloats(arr: number[]): string {
  return arr.map((coord) => coord.toFixed(3)).join(', ');
}

export namespace FxdkMenus {
  export const FXDK = [...MAIN_MENU_BAR, '1_fxdk'];
  export const FXDK_GAME_INSERTIONS = [...FXDK, '1_insertions'];
  export const FXDK_TOGGLES = [...FXDK, '2_toggles'];
  export const FXDK_COMMANDS = [...FXDK, '3_fxdk'];

  export const GAME_CONTEXT = [...EDITOR_CONTEXT_MENU, '0_game_context'];
  export const GAME_CONTEXT_INSERTIONS = [...GAME_CONTEXT, 'z_game_insertions'];
}

export namespace FxdkCommands {
  const FXDK_CATEGORY = 'FxDK';

  export const INSERT_CURRENT_POS: Command = {
    id: 'fxdk.insertCurrentPos',
    category: FXDK_CATEGORY,
    label: 'Insert player position',
  };
  export const INSERT_CURRENT_ROT: Command = {
    id: 'fxdk.insertCurrentRot',
    category: FXDK_CATEGORY,
    label: 'Insert player rotation',
  };
  export const INSERT_CURRENT_HEADING: Command = {
    id: 'fxdk.insertCurrentHeading',
    category: FXDK_CATEGORY,
    label: 'Insert player heading',
  };

  export const START_SERVER: Command = {
    id: 'fxdk.startServer',
    category: FXDK_CATEGORY,
    label: 'Start game server',
  };
  export const BUILD_PROJECT: Command = {
    id: 'fxdk.buildProject',
    category: FXDK_CATEGORY,
    label: 'Build project',
  };
}

@injectable()
export class FxdkMenuContribution implements MenuContribution, CommandContribution {
  @inject(FxdkDataService)
  private readonly dataService: FxdkDataService;

  @inject(EditorManager)
  private readonly editorManager: EditorManager;

  registerMenus(registry: MenuModelRegistry): void {
    /**
     * Bar menus
     */
    registry.registerSubmenu(FxdkMenus.FXDK, 'FxDK');

    // Insertions
    {
      registry.registerMenuAction(FxdkMenus.FXDK_GAME_INSERTIONS, {
        commandId: FxdkCommands.INSERT_CURRENT_POS.id,
      });
      registry.registerMenuAction(FxdkMenus.FXDK_GAME_INSERTIONS, {
        commandId: FxdkCommands.INSERT_CURRENT_ROT.id,
      });
      registry.registerMenuAction(FxdkMenus.FXDK_GAME_INSERTIONS, {
        commandId: FxdkCommands.INSERT_CURRENT_HEADING.id,
      });
    }

    // Toggles
    {
      registry.registerMenuAction(FxdkMenus.FXDK_TOGGLES, {
        commandId: 'fxdkGameView:toggle',
        label: 'Toggle Game View',
        icon: 'fa fa-gamepad',
      });
      registry.registerMenuAction(FxdkMenus.FXDK_TOGGLES, {
        commandId: ServerConsoleViewContribution.TOGGLE_COMMAND_ID,
        label: 'Toggle Server Console',
        icon: SERVER_CONSOLE_WIDGET_ICON,
      });
      registry.registerMenuAction(FxdkMenus.FXDK_TOGGLES, {
        commandId: ClientConsoleViewContribution.TOGGLE_COMMAND_ID,
        label: 'Toggle Client Console',
        icon: CLIENT_CONSOLE_WIDGET_ICON,
      });
    }

    // FxDK Commands
    {
      registry.registerMenuAction(FxdkMenus.FXDK_COMMANDS, {
        commandId: FxdkCommands.START_SERVER.id,
        label: 'Start Game Server',
        icon: 'fa fa-play',
      });
      registry.registerMenuAction(FxdkMenus.FXDK_COMMANDS, {
        commandId: FxdkCommands.BUILD_PROJECT.id,
        label: 'Build Project',
        icon: 'fa fa-download',
      });
    }

    /**
     * Context menus
     */
    registry.registerMenuAction(FxdkMenus.GAME_CONTEXT_INSERTIONS, {
      commandId: FxdkCommands.INSERT_CURRENT_POS.id,
      icon: 'fa fa-gamepad',
    });
    registry.registerMenuAction(FxdkMenus.GAME_CONTEXT_INSERTIONS, {
      commandId: FxdkCommands.INSERT_CURRENT_ROT.id,
      icon: 'fa fa-gamepad',
    });
  }

  registerCommands(registry: CommandRegistry): void {
    registry.registerCommand(FxdkCommands.INSERT_CURRENT_POS, this.newEditorCommandHandler((editor: TextEditor) => {
      const pos = this.dataService.data['player_ped_pos'];
      if (!pos) {
        console.log('No pos yet');
        return;
      }

      editor.executeEdits([{
        range: editor.selection,
        newText: formatArrayOfFloats(pos),
      }]);
    }));

    registry.registerCommand(FxdkCommands.INSERT_CURRENT_ROT, this.newEditorCommandHandler((editor: TextEditor) => {
      const rot = this.dataService.data['player_ped_rot'];
      if (!rot) {
        console.log('No rot yet');
        return;
      }

      editor.executeEdits([{
        range: editor.selection,
        newText: formatArrayOfFloats(rot),
      }]);
    }));

    registry.registerCommand(FxdkCommands.INSERT_CURRENT_HEADING, this.newEditorCommandHandler((editor: TextEditor) => {
      const heading = this.dataService.data['player_ped_heading'];
      if (!heading) {
        console.log('No heading yet');
        return;
      }

      editor.executeEdits([{
        range: editor.selection,
        newText: heading.toFixed(3),
      }]);
    }));

    registry.registerCommand(FxdkCommands.START_SERVER, {
      execute: () => this.dataService.sendMessageToShell('fxdk:startServer'),
    });
    registry.registerCommand(FxdkCommands.BUILD_PROJECT, {
      execute: () => this.dataService.sendMessageToShell('fxdk:buildProject'),
    });
  }

  private newCommandHandler(handler: Function): FxdkMenuCommandHandler {
    return new FxdkMenuCommandHandler(handler);
  }

  private newEditorCommandHandler(handler: (editor: TextEditor) => void): FxdkMenuCommandHandler {
    const commandHandler = () => {
      const editor = this.editorManager.activeEditor?.editor;
      if (!editor) {
        console.log('No active editor');
        return;
      }

      handler(editor);
    };

    return this.newCommandHandler(commandHandler);
  }
}

export class FxdkMenuCommandHandler implements CommandHandler {
  constructor(
    private handler: Function,
  ) {
  }

  execute(...args: any[]): any {
    this.handler(...args);
  }
}
