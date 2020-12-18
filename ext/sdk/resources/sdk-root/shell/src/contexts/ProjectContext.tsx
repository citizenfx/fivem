import React from 'react';
import { FilesystemEntry, ProjectData, RecentProject, AppStates, ProjectFsUpdate, ProjectResources } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { getProjectResources } from 'shared/utils';
import { sendApiMessage } from 'utils/api';
import { useApiMessage, useOpenFlag } from 'utils/hooks';
import { StateContext } from './StateContext';
import { TheiaContext } from './TheiaContext';
import { logger } from 'utils/logger';

const log = logger('ProjectContext');
export interface ProjectContext {
  creatorOpen: boolean,
  openCreator: () => void,
  closeCreator: () => void,

  openerOpen: boolean,
  openOpener: () => void,
  closeOpener: () => void,

  assetCreatorDir: string,
  setAssetCreatorDir: (dir: string) => void,

  assetCreatorOpen: boolean,
  openAssetCreator: () => void,
  closeAssetCreator: () => void,

  directoryCreatorOpen: boolean,
  openDirectoryCreator: () => void,
  closeDirectoryCreator: () => void,

  openProject: (string) => void,
  project: ProjectData | null,
  recentProjects: RecentProject[],

  openFile: (entry: FilesystemEntry) => void,
}

export const ProjectContext = React.createContext<ProjectContext>({
  creatorOpen: false,
  openCreator: () => { },
  closeCreator: () => { },

  openerOpen: false,
  openOpener: () => { },
  closeOpener: () => { },

  assetCreatorDir: '',
  setAssetCreatorDir: () => { },

  assetCreatorOpen: false,
  openAssetCreator: () => { },
  closeAssetCreator: () => { },

  directoryCreatorOpen: false,
  openDirectoryCreator: () => { },
  closeDirectoryCreator: () => { },

  openProject: () => { },
  project: null,
  recentProjects: [],

  openFile: () => { },
});

export const ProjectContextProvider = React.memo(function ProjectContextProvider({ children }) {
  const { state } = React.useContext(StateContext);
  const { openFileInTheia } = React.useContext(TheiaContext);

  const [creatorOpen, openCreator, closeCreator] = useOpenFlag(false);
  const [openerOpen, openOpener, closeOpener] = useOpenFlag(false);

  const [project, setProject] = React.useState<ProjectData | null>(null);

  const [recentProjects, setRecentProjects] = React.useState<RecentProject[]>([]);

  const [assetCreatorDir, setAssetCreatorDir] = React.useState('');
  const [assetCreatorOpen, openAssetCreator, closeAssetCreator] = useOpenFlag(false);
  const [directoryCreatorOpen, openDirectoryCreator, closeDirectoryCreator] = useOpenFlag(false);

  const projectRef = React.useRef<ProjectData | null>(null);
  projectRef.current = project;

  const projectOpenPendingRef = React.useRef(false);

  const openProject = React.useCallback((path: string) => {
    if (!projectOpenPendingRef.current) {
      projectOpenPendingRef.current = true;
      sendApiMessage(projectApi.open, path);
    }
  }, []);

  const openFile = React.useCallback((entry: FilesystemEntry) => {
    if (project) {
      openFileInTheia(entry.path);
    }
  }, [project, openFileInTheia]);

  React.useEffect(() => {
    if (state === AppStates.ready) {
      sendApiMessage(projectApi.getRecents);
    }
  }, [state]);

  useApiMessage(projectApi.open, (project) => {
    projectOpenPendingRef.current = false;
    setProject(project);
    localStorage.setItem('last-project-path', project.path);
  }, [setProject]);

  useApiMessage(projectApi.close, () => {
    setProject(null);
  }, [setProject]);

  useApiMessage(projectApi.update, (updatedProject) => {
    setProject({ ...projectRef.current, ...updatedProject });
  }, [setProject]);

  // Handle fs update
  useApiMessage(projectApi.fsUpdate, (update: ProjectFsUpdate) => {
    const newProject = { ...projectRef.current };

    log('Processing fs update', update);

    if (update.replace) {
      Object.entries(update.replace).forEach(([key, value]) => {
        newProject.fs[key] = value;
      });
    }

    if (update.delete) {
      update.delete.forEach((key) => {
        delete newProject.fs[key];
      });
    }

    log('New project state', newProject);

    setProject(newProject);
  }, [setProject]);

  // Handle resources update
  useApiMessage(projectApi.resourcesUpdate, (resources: ProjectResources) => {
    const newProject = { ...projectRef.current };

    log('Processing resources update', resources);

    newProject.resources = resources;

    setProject(newProject);
  }, [setProject]);

  useApiMessage(projectApi.recents, (recentProjects: RecentProject[]) => {
    setRecentProjects(recentProjects);

    const lastProjectPath = localStorage.getItem('last-project-path');

    // Only open matching last recent project if no project open
    if (!project) {
      const [lastProject] = recentProjects;

      if (lastProject && lastProject.path === lastProjectPath) {
        openProject(lastProject.path);
      }
    }
  }, [project, openProject]);

  const value = {
    creatorOpen,
    openCreator,
    closeCreator,

    openerOpen,
    openOpener,
    closeOpener,

    assetCreatorDir,
    setAssetCreatorDir,

    assetCreatorOpen,
    openAssetCreator,
    closeAssetCreator,

    directoryCreatorOpen,
    openDirectoryCreator,
    closeDirectoryCreator,

    openProject,
    project,
    recentProjects,

    openFile,
  };

  return (
    <ProjectContext.Provider value={value}>
      {children}
    </ProjectContext.Provider>
  );
});
