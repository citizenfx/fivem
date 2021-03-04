export const openInExplorer = (url: string): boolean => invokeNative('openUrl', url) as any;

export const openInExplorerAndSelect = (url: string): boolean => invokeNative('openFolderAndSelectFile', url) as any;
