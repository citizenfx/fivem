import React from 'react';
import { FilesystemEntry, ProjectData, RecentProject, AppStates, ProjectFsUpdate, ProjectResources } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useApiMessage, useOpenFlag } from 'utils/hooks';
import { StateContext } from './StateContext';
import { TheiaContext } from './TheiaContext';
import { logger } from 'utils/logger';
import { ProjectBuildRequest } from 'shared/api.requests';
import { getProjectBuildPathVar, getProjectClientStorageItem, getProjectDeployArtifactVar, getProjectSteamWebApiKeyVar, getProjectTebexSecretVar, getProjectUseVersioningVar } from 'utils/projectStorage';

const log = logger('ProjectContext');
export interface ProjectContext {
  creatorOpen: boolean,
  openCreator: () => void,
  closeCreator: () => void,

  openerOpen: boolean,
  openOpener: () => void,
  closeOpener: () => void,

  settingsOpen: boolean,
  openSettings: () => void,
  closeSettings: () => void,

  build: (overrides?: Partial<ProjectBuildRequest>) => void,
  builderOpen: boolean,
  openBuilder: () => void,
  closeBuilder: () => void,

  resourceCreatorDir: string,
  setResourceCreatorDir: (dir: string) => void,

  resourceCreatorOpen: boolean,
  openResourceCreator: () => void,
  closeResourceCreator: () => void,

  directoryCreatorOpen: boolean,
  openDirectoryCreator: () => void,
  closeDirectoryCreator: () => void,

  openProject: (string) => void,
  project: ProjectData | null,
  projectEntry: FilesystemEntry | null,
  recentProjects: RecentProject[],

  pendingDirectoryDeletions: Set<string>,
  addPendingDirectoryDeletion: (directoryPath: string) => void,

  openFile: (entry: FilesystemEntry) => void,
}

export const ProjectContext = React.createContext<ProjectContext>({
  creatorOpen: false,
  openCreator: () => { },
  closeCreator: () => { },

  openerOpen: false,
  openOpener: () => { },
  closeOpener: () => { },

  settingsOpen: false,
  openSettings: () => { },
  closeSettings: () => { },

  build: () => { },
  builderOpen: false,
  openBuilder: () => { },
  closeBuilder: () => { },

  resourceCreatorDir: '',
  setResourceCreatorDir: () => { },

  resourceCreatorOpen: false,
  openResourceCreator: () => { },
  closeResourceCreator: () => { },

  directoryCreatorOpen: false,
  openDirectoryCreator: () => { },
  closeDirectoryCreator: () => { },

  openProject: () => { },
  project: null,
  projectEntry: null,
  recentProjects: [],

  pendingDirectoryDeletions: new Set(),
  addPendingDirectoryDeletion: () => {},

  openFile: () => { },
});

export const ProjectContextProvider = React.memo(function ProjectContextProvider({ children }) {
  const { state } = React.useContext(StateContext);
  const { openFileInTheia } = React.useContext(TheiaContext);

  const [creatorOpen, openCreator, closeCreator] = useOpenFlag(false);
  const [openerOpen, openOpener, closeOpener] = useOpenFlag(false);

  const [project, setProject] = React.useState<ProjectData | null>(null);

  const [settingsOpen, openSettings, closeSettings] = useOpenFlag(false);
  const [builderOpen, openBuilder, closeBuilder] = useOpenFlag(false);

  const [recentProjects, setRecentProjects] = React.useState<RecentProject[]>([]);

  const [resourceCreatorDir, setResourceCreatorDir] = React.useState('');
  const [resourceCreatorOpen, openResourceCreator, closeResourceCreator] = useOpenFlag(false);

  const [directoryCreatorOpen, openDirectoryCreator, closeDirectoryCreator] = useOpenFlag(false);

  const [pendingDirectoryDeletions, setPendingDirectoryDeletions] = React.useState(new Set<string>());

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

  useApiMessage(projectApi.freePendingFolderDeletion, (directoryPath: string) => {
    if (pendingDirectoryDeletions.has(directoryPath)) {
      const newPendingDeletions = new Set(pendingDirectoryDeletions);

      newPendingDeletions.delete(directoryPath);

      setPendingDirectoryDeletions(newPendingDeletions);
    }
  }, [pendingDirectoryDeletions, setPendingDirectoryDeletions]);

  const addPendingDirectoryDeletion = React.useCallback((directoryPath: string) => {
    const newPendingDeletions = new Set(pendingDirectoryDeletions);

    newPendingDeletions.add(directoryPath);

    setPendingDirectoryDeletions(newPendingDeletions);
  }, [pendingDirectoryDeletions, setPendingDirectoryDeletions]);

  const projectEntry = React.useMemo((): FilesystemEntry | null => {
    if (!project) {
       return null;
    }

    return {
      path: project.path,
      name: project.path.substr(
        Math.max(
          Math.max(
            project.path.lastIndexOf('/'),
            project.path.lastIndexOf('\\'),
          ) + 1,
          0,
        ),
      ),
      isDirectory: true,
      isFile: false,
      isSymbolicLink: false,
      meta: {
        isFxdkProject: true,
        isResource: false,
        assetMeta: null,
      },
    };
  }, [project?.path]);

  const build = React.useCallback((overrides?: Partial<ProjectBuildRequest>) => {
    if (!project) {
      return;
    }

    const buildPath = getProjectBuildPathVar(project);
    const useVersioning = getProjectUseVersioningVar(project);
    const deployArtifact = getProjectDeployArtifactVar(project);
    const steamWebApiKey = getProjectSteamWebApiKeyVar(project);
    const tebexSecret = getProjectTebexSecretVar(project);

    if (!buildPath) {
      return openBuilder();
    }

    sendApiMessage(projectApi.build, {
      buildPath,
      useVersioning,
      deployArtifact,
      steamWebApiKey,
      tebexSecret,
      ...overrides,
    } as ProjectBuildRequest);
  }, [project, openBuilder]);

  const value: ProjectContext = {
    creatorOpen,
    openCreator,
    closeCreator,

    openerOpen,
    openOpener,
    closeOpener,

    settingsOpen,
    openSettings,
    closeSettings,

    build,
    builderOpen,
    openBuilder,
    closeBuilder,

    resourceCreatorDir,
    setResourceCreatorDir,

    resourceCreatorOpen,
    openResourceCreator,
    closeResourceCreator,

    directoryCreatorOpen,
    openDirectoryCreator,
    closeDirectoryCreator,

    openProject,
    project,
    projectEntry,
    recentProjects,

    pendingDirectoryDeletions,
    addPendingDirectoryDeletion,

    openFile,
  };

  return (
    <ProjectContext.Provider value={value}>
      {children}
    </ProjectContext.Provider>
  );
});
