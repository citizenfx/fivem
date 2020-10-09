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
  BsPlayFill
} from 'react-icons/bs';

export const devtoolsIcon = <BsTerminal />;

export const projectConfigIcon = <BsGear />;
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

export const deleteIcon = <BsTrash />;
export const renameIcon = <BsPencil />;

export const RefreshIconComponent = BsArrowRepeat;
export const refreshIcon = <RefreshIconComponent />;
export const rotatingRefreshIcon = <RefreshIconComponent className="rotating" />;

export const stopIcon = <BsStopFill />;
export const startIcon = <BsPlayFill />;
