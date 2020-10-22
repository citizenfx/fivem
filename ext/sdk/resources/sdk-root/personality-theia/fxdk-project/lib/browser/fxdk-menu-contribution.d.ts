import { Command, CommandContribution, CommandHandler, CommandRegistry, MenuContribution, MenuModelRegistry } from '@theia/core';
export declare namespace FxdkMenus {
    const GAME: string[];
    const GAME_INSERTIONS: string[];
    const GAME_TOGGLES: string[];
    const GAME_CONTEXT: string[];
    const GAME_CONTEXT_INSERTIONS: string[];
}
export declare namespace FxdkCommands {
    const INSERT_CURRENT_POS: Command;
    const INSERT_CURRENT_ROT: Command;
    const INSERT_CURRENT_HEADING: Command;
}
export declare class FxdkMenuContribution implements MenuContribution, CommandContribution {
    private readonly dataService;
    private readonly editorManager;
    registerMenus(registry: MenuModelRegistry): void;
    registerCommands(registry: CommandRegistry): void;
    private newCommandHandler;
    private newEditorCommandHandler;
}
export declare class FxdkMenuCommandHandler implements CommandHandler {
    private handler;
    constructor(handler: Function);
    execute(...args: any[]): any;
}
//# sourceMappingURL=fxdk-menu-contribution.d.ts.map