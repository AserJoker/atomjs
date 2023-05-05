import { getVersion } from "./pack/demo";
import { print } from "core";
import { Data } from "io";
const data = new Data(123);
print(`hello world ${data.getId()}`);
