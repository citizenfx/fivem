import React from 'react';
import { WorldEditorState } from './WorldEditorState';

export const useInputOverride = () => React.useEffect(() => WorldEditorState.overrideInput(), []);
