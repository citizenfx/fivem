import React from 'react';
import { FilesystemEntry, Project, ProjectResources, RecentProject, States } from 'sdkApi/api.types';
import { projectApi } from 'sdkApi/events';
import { getProjectResources } from 'sdkApi/utils';
import { sendApiMessage } from 'utils/api';
import { useApiMessage, useOpenFlag } from 'utils/hooks';
import { getFoldersForTheia } from 'utils/project';
import { StateContext } from './StateContext';
import { TheiaContext } from './TheiaContext';


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
  project: Project | null,
  projectResources: ProjectResources,
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
  projectResources: {},
  recentProjects: [],

  openFile: () => { },
});

export const ProjectContextProvider = React.memo(function ProjectContextProvider({ children }) {
  const { state } = React.useContext(StateContext);
  const { theiaIsReady, openProjectInTheia, openFileInTheia } = React.useContext(TheiaContext);

  const [creatorOpen, openCreator, closeCreator] = useOpenFlag(false);
  const [openerOpen, openOpener, closeOpener] = useOpenFlag(false);

  const [project, setProject] = React.useState<Project | null>(null);
  const [recentProjects, setRecentProjects] = React.useState<RecentProject[]>([]);

  const [assetCreatorDir, setAssetCreatorDir] = React.useState('');
  const [assetCreatorOpen, openAssetCreator, closeAssetCreator] = useOpenFlag(false);
  const [directoryCreatorOpen, openDirectoryCreator, closeDirectoryCreator] = useOpenFlag(false);

  const projectRef = React.useRef<Project | null>(null);
  projectRef.current = project;

  const projectOpenPendingRef = React.useRef(false);
  const theiaFoldersStringRef = React.useRef('');

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
    if (state === States.ready) {
      sendApiMessage(projectApi.getRecents);
    }
  }, [state]);

  // Handling project change so we open that in theia correctly
  React.useEffect(() => {
    if (project && theiaIsReady) {
      const folders = getFoldersForTheia(project);
      const theiaFoldersString = JSON.stringify(folders);

      if (theiaFoldersStringRef.current !== theiaFoldersString) {
        theiaFoldersStringRef.current = theiaFoldersString;

        openProjectInTheia({
          name: project.manifest.name,
          path: project.path,
          folders,
        });
      }
    }
  }, [project, theiaIsReady, openProjectInTheia]);

  useApiMessage(projectApi.open, (project) => {
    projectOpenPendingRef.current = false;
    setProject(project);
  }, [setProject]);

  useApiMessage(projectApi.update, (updatedProject) => {
    setProject({ ...projectRef.current, ...updatedProject });
  }, [setProject]);

  useApiMessage(projectApi.recents, (recentProjects: RecentProject[]) => {
    setRecentProjects(recentProjects);

    // Only open last recent project if no project open
    if (!project) {
      const [lastProject] = recentProjects;

      if (lastProject) {
        openProject(lastProject.path);
      }
    }
  }, [project, openProject]);

  const projectResources = React.useMemo(() => {
    if (!project) {
      return {};
    }

    return getProjectResources(project);
  }, [project]);

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
    projectResources,
    recentProjects,

    openFile,
  };

  return (
    <ProjectContext.Provider value={value}>
      {children}
    </ProjectContext.Provider>
  );
});
