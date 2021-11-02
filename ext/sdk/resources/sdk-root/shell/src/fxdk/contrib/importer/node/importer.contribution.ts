import { registerApiContribution } from "backend/api/api.extensions";
import { registerSingleton } from "backend/container-access";
import { ExamplesImporter } from "./importer.examples";
import { FsImporter } from "./importer.fs";
import { GitImporter } from "./importer.git";

registerApiContribution(
  registerSingleton(GitImporter)
);

registerApiContribution(
  registerSingleton(FsImporter)
);

registerApiContribution(
  registerSingleton(ExamplesImporter)
);
