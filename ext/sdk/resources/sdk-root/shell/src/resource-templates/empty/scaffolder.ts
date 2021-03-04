import { injectable } from "inversify";
import { ResourceTemplateScaffolder } from "../types";

@injectable()
export default class EmptyScaffolder implements ResourceTemplateScaffolder {
  scaffold() {
  }
}
