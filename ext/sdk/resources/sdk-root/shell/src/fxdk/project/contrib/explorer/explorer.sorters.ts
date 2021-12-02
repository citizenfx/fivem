import { IFsEntry } from 'fxdk/project/common/project.types';
import { alphabeticalOrderCollator } from 'fxdk/base/collators';

export const entriesSorter = (a: IFsEntry, b: IFsEntry) => {
  if (a.isDirectory && !b.isDirectory) {
    return -1;
  }

  if (!a.isDirectory && b.isDirectory) {
    return 1;
  }

  return alphabeticalOrderCollator.compare(a.name, b.name);
};
