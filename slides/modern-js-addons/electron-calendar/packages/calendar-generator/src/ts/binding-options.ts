import { Options } from 'pkg-prebuilds';
import * as meta from '../../package.json';

const options: Options = {
  name: meta.name,
  napi_versions: meta.binary.napi_versions,
};

export = options;
