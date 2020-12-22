import { changelogEntries } from "./Changelog.entries";

export const getLatestChangelogEntry = () => {
  return changelogEntries[0];
};

export const getLatestSeenChangelogEntryId = () => {
  return localStorage['last-changelog-id'];
};

export const setLatestSeenChangelogEntryId = (id: string) => {
  localStorage['last-changelog-id'] = id;
};

export const setLatestChangelogEntryAsSeen = () => {
  setLatestSeenChangelogEntryId(getLatestChangelogEntry().id);
};

export const hasNewChangelogEntries = (): boolean => {
  return getLatestSeenChangelogEntryId() !== getLatestChangelogEntry().id;
};
