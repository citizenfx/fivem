export type ArchetypeFileIndex = number;
export type ArchetypeLodDist = number;
export type ArchetypeName = string;

export interface Archetypes {
  files: string[],
  archetypes: {
    [key: ArchetypeName]: [
      ArchetypeFileIndex,
      ArchetypeLodDist,
    ],
  },
}
