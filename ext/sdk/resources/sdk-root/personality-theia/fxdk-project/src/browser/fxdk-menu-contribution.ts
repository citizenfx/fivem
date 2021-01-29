import { Command, CommandContribution, CommandHandler, CommandRegistry, MAIN_MENU_BAR, MenuContribution, MenuModelRegistry } from '@theia/core';
import { EditorManager } from '@theia/editor/lib/browser/editor-manager';
import { EDITOR_CONTEXT_MENU } from '@theia/editor/lib/browser/editor-menu';
import { TextEditor } from '@theia/editor/lib/browser/editor';
import { injectable, inject } from 'inversify';
import { FxdkDataService } from './fxdk-data-service';
import { ServerConsoleViewContribution, SERVER_CONSOLE_WIDGET_ICON } from './console/server-console';

function formatArrayOfFloats(arr: number[]): string {
  return arr.map((coord) => coord.toFixed(3)).join(', ');
}

export namespace FxdkMenus {
  export const GAME = [...MAIN_MENU_BAR, '1_game'];
  export const GAME_INSERTIONS = [...GAME, '1_insertions'];
  export const GAME_TOGGLES = [...GAME, '2_toggles'];

  export const GAME_CONTEXT = [...EDITOR_CONTEXT_MENU, '0_game_context'];
  export const GAME_CONTEXT_INSERTIONS = [...GAME_CONTEXT, 'z_game_insertions'];
}

export namespace FxdkCommands {
  const GAME_CATEGORY = 'Game';

  export const INSERT_CURRENT_POS: Command = {
    id: 'fxdk.insertCurrentPos',
    category: GAME_CATEGORY,
    label: 'Insert player position',
  };
  export const INSERT_CURRENT_ROT: Command = {
    id: 'fxdk.insertCurrentRot',
    category: GAME_CATEGORY,
    label: 'Insert player rotation',
  };
  export const INSERT_CURRENT_HEADING: Command = {
    id: 'fxdk.insertCurrentHeading',
    category: GAME_CATEGORY,
    label: 'Insert player heading',
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
    registry.registerSubmenu(FxdkMenus.GAME, 'Game');

    registry.registerMenuAction(FxdkMenus.GAME_INSERTIONS, {
      commandId: FxdkCommands.INSERT_CURRENT_POS.id,
    });
    registry.registerMenuAction(FxdkMenus.GAME_INSERTIONS, {
      commandId: FxdkCommands.INSERT_CURRENT_ROT.id,
    });
    registry.registerMenuAction(FxdkMenus.GAME_INSERTIONS, {
      commandId: FxdkCommands.INSERT_CURRENT_HEADING.id,
    });

    registry.registerMenuAction(FxdkMenus.GAME_TOGGLES, {
      commandId: 'fxdkGameView:toggle',
      label: 'Toggle Game View',
      icon: 'fa fa-gamepad',
    });
    registry.registerMenuAction(FxdkMenus.GAME_TOGGLES, {
      commandId: ServerConsoleViewContribution.TOGGLE_COMMAND_ID,
      label: 'Toggle Server Console',
      icon: SERVER_CONSOLE_WIDGET_ICON,
    });

    /**
     * Context menus
     */
    registry.registerMenuAction(FxdkMenus.GAME_CONTEXT_INSERTIONS, {
      commandId: FxdkCommands.INSERT_CURRENT_POS.id,
    });
    registry.registerMenuAction(FxdkMenus.GAME_CONTEXT_INSERTIONS, {
      commandId: FxdkCommands.INSERT_CURRENT_ROT.id,
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
