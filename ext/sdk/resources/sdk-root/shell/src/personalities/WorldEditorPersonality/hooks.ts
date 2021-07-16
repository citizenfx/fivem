import React from 'react';
import { WEState } from './store/WEState';

export const useInputOverride = () => React.useEffect(() => WEState.overrideInput(), []);
