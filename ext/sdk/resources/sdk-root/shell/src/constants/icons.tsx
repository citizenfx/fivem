import React from 'react';
import {
  BsDiamond,
  BsFolderFill,
  BsFolder,
  BsLayersFill,
  BsPlusSquare,
  BsFolderPlus,
  BsFileEarmark,
  BsPuzzle,
  BsTrash,
  BsGear,
  BsTerminal,
  BsLayersHalf,
  BsArrowRepeat,
  BsPencil,
  BsDiamondFill,
  BsFileEarmarkPlus,
  BsStopFill,
  BsPlayFill,
  BsX,
  BsBoxArrowInDown,
  BsBoxArrowUpRight,
  BsMap,
  BsBoxArrowInDownRight
} from 'react-icons/bs';
import { VscSettingsGear } from 'react-icons/vsc';

export const devtoolsIcon = <BsTerminal />;

export const projectBuildIcon = <BsBoxArrowInDown />;

export const openInExplorerIcon = <BsBoxArrowUpRight />;

export const projectSettingsIcon = <VscSettingsGear />;
export const newProjectIcon = <BsLayersFill />;
export const openProjectIcon = <BsLayersHalf />;

export const resourceIcon = <BsDiamond />;
export const disabledResourceIcon = resourceIcon;
export const enabledResourceIcon = <BsDiamondFill />;
export const projectIcon = <BsLayersFill />;
export const assetIcon = <BsPuzzle />;
export const fileIcon = <BsFileEarmark />;

export const openDirectoryIcon = <BsFolder />;
export const closedDirectoryIcon = <BsFolderFill />;

export const newFileIcon = <BsFileEarmarkPlus />;
export const newResourceIcon = <BsPlusSquare />;
export const newDirectoryIcon = <BsFolderPlus />;
export const importAssetIcon = <BsBoxArrowInDownRight />;

export const deleteIcon = <BsTrash />;
export const renameIcon = <BsPencil />;

export const RefreshIconComponent = BsArrowRepeat;
export const refreshIcon = <RefreshIconComponent />;
export const rotatingRefreshIcon = <RefreshIconComponent className="rotating" />;

export const stopIcon = <BsStopFill />;
export const startIcon = <BsPlayFill />;

export const closeIcon = <BsX />;
export const mapIcon = <BsMap />;
