import { injectable } from "inversify";
import { UpdaterContribution } from "./updater-contribution";

@injectable()
export class UpdaterDummyContribution implements UpdaterContribution {
  async update() {

  }
}
