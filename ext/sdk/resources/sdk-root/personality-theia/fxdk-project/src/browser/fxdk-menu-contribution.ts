import { Command, CommandContribution, CommandHandler, CommandRegistry, MAIN_MENU_BAR, MenuContribution, MenuModelRegistry } from '@theia/core';
import { EditorManager } from '@theia/editor/lib/browser/editor-manager';
import { EDITOR_CONTEXT_MENU } from '@theia/editor/lib/browser/editor-menu';
import { injectable, inject } from 'inversify';
import { FxdkDataService } from './fxdk-data-service';

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

    registry.registerMenuAction(FxdkMenus.GAME_TOGGLES, {
      commandId: 'fxdkGameView:toggle',
      label: 'Toggle Game View',
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
