import { Command, CommandContribution, CommandHandler, CommandRegistry, MAIN_MENU_BAR, MenuContribution, MenuModelRegistry } from '@theia/core';
import { EditorManager } from '@theia/editor/lib/browser/editor-manager';
import { injectable, inject } from 'inversify';
import { FxdkDataService } from './fxdk-data-service';

export namespace FxdkMenus {
  export const GAME = [...MAIN_MENU_BAR, '1_game'];
  export const GAME_TOGGLE_VIEW = [...GAME, '1_toggle_view'];
  export const GAME_INSERT_CURRENT_POS = [...GAME, '2_insert_current_pos'];
  export const GAME_INSERT_CURRENT_ROT = [...GAME, '3_insert_current_rot'];
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
}

@injectable()
export class FxdkMenuContribution implements MenuContribution, CommandContribution {
  @inject(FxdkDataService)
  private readonly dataService: FxdkDataService;

  @inject(EditorManager)
  private readonly editorManager: EditorManager;

  registerMenus(registry: MenuModelRegistry): void {
    registry.registerSubmenu(FxdkMenus.GAME, 'Game');

    registry.registerMenuAction(FxdkMenus.GAME_TOGGLE_VIEW, {
      commandId: 'fxdkGameView:toggle',
      label: 'Toggle Game View',
    });
    registry.registerMenuAction(FxdkMenus.GAME_INSERT_CURRENT_POS, {
      commandId: FxdkCommands.INSERT_CURRENT_POS.id,
    });
    registry.registerMenuAction(FxdkMenus.GAME_INSERT_CURRENT_ROT, {
      commandId: FxdkCommands.INSERT_CURRENT_ROT.id,
    });
  }

  registerCommands(registry: CommandRegistry): void {
    registry.registerCommand(FxdkCommands.INSERT_CURRENT_POS, this.newCommandHandler(() => {
      const editor = this.editorManager.activeEditor?.editor;
      if (!editor) {
        console.log('No active editor');
        return;
      }

      editor.executeEdits([{
        range: editor.selection,
        newText: this.dataService.data['player_ped_pos'] + '',
      }]);
    }));
    registry.registerCommand(FxdkCommands.INSERT_CURRENT_ROT, this.newCommandHandler(() => {
      const editor = this.editorManager.activeEditor?.editor;
      if (!editor) {
        console.log('No active editor');
        return;
      }

      editor.executeEdits([{
        range: editor.selection,
        newText: this.dataService.data['player_ped_rot'] + '',
      }]);
    }));
  }

  private newCommandHandler(handler: Function): FxdkMenuCommandHandler {
    return new FxdkMenuCommandHandler(handler);
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
