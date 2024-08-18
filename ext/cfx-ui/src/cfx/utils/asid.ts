import { typeid } from 'typeid-js';

function generateNewASID() {
  return typeid('asid').toString();
}

namespace LS_KEY {
  export const ASID = 'anonymousStableIdentifier';
}

/**
 * ASID stands for Anonymous Stable Identifier
 *
 * It holds no connection to user accounts by itself and only needed for grouping telemetry for one particular user
 */
export const ASID = (() => {
  try {
    const existingASID = window.localStorage.getItem(LS_KEY.ASID);

    if (existingASID) {
      return existingASID;
    }

    const newASID = generateNewASID();

    window.localStorage.setItem(LS_KEY.ASID, newASID);

    return newASID;
  } catch (e) {
    if (__CFXUI_DEV__) {
      console.error(e);
    }

    return generateNewASID();
  }
})();

if (__CFXUI_DEV__) {
  console.log('Current ASID:', ASID);
}
