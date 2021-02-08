import { useState, useEffect, useCallback, useMemo } from 'react';
import { ProjectData } from 'shared/api.types';

function composeKey(project: ProjectData | null, key: string): string {
  return `${project?.path || '__noproject'}:${key}`;
}

export function getProjectClientStorageItem<ValueType>(project: ProjectData | null, key: string, defaultValue: ValueType): ValueType {
  const composedKey = composeKey(project, key);

  if (window.localStorage[composedKey]) {
    return JSON.parse(window.localStorage[composedKey]);
  }

  return defaultValue;
}

export function useProjectClientStorageItem<ValueType>(project: ProjectData | null, key: string, defaultValue: ValueType): [ValueType, (newValue: ValueType) => void] {
  const initialValue = useMemo(() => getProjectClientStorageItem(project, key, defaultValue), [project?.path, key]);

  const [value, setValue] = useState(initialValue);
  const composedKey = useMemo(() => composeKey(project, key), [project?.path, key]);

  const handleSetValue = useCallback((newValue: ValueType) => {
    window.localStorage[composedKey] = JSON.stringify(newValue);
    setValue(newValue);
  }, [setValue, composedKey]);

  return [value, handleSetValue];
}
