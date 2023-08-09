import { fastRandomId } from "./random";

module LS_KEY {
  export const ASID = 'anonymousStableIdentifier';
  export const SentryUserId = 'sentryUserId';
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
      // Clean up the sentry user id if it still exists
      window.localStorage.removeItem(LS_KEY.SentryUserId);

      return existingASID;
    }

    // May be migrate from sentry user id as it is literally the same as ASID
    const sentryUserId = window.localStorage.getItem(LS_KEY.SentryUserId);
    if (sentryUserId) {
      window.localStorage.setItem(LS_KEY.ASID, sentryUserId);
      window.localStorage.removeItem(LS_KEY.SentryUserId);

      return sentryUserId;
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

function generateNewASID() {
  return `${fastRandomId()}-${fastRandomId()}`;
}
