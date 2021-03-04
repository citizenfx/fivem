export class AssetBuildCommandError {
  constructor(
    public readonly assetName: string,
    public readonly outputChannelId: string,
  ) { }
}
