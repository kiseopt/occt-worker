import { readFile, writeFile } from "node:fs/promises";
import path from "node:path";

const args = process.argv.slice(2);
const mapPath = args[0];
const outputIndex = args.indexOf("--output");
const outputPath = outputIndex === -1 ? undefined : args[outputIndex + 1];

if (mapPath === undefined || (outputIndex !== -1 && outputPath === undefined)) {
  throw new Error("Usage: node scripts/report-occt-link-map.mjs <link-map> [--output <report.md>]");
}

const map = await readFile(mapPath, "utf8");
const sizes = new Map();
for (const line of map.split(/\r?\n/)) {
  const contribution = line.match(/^\s+\S+\s+\S+\s+([0-9a-fA-F]+)\s+(.+)$/);
  if (contribution === null) continue;
  const archive = contribution[2].match(/[\\/](lib(TK[^\\/()]+)\.a)\(/);
  if (archive === null) continue;
  sizes.set(archive[2], (sizes.get(archive[2]) ?? 0) + Number.parseInt(contribution[1], 16));
}

if (sizes.size === 0) {
  throw new Error(`No OCCT archive contributions found in '${mapPath}'`);
}

const rows = [...sizes.entries()].sort((left, right) => right[1] - left[1]);
const total = rows.reduce((sum, [, bytes]) => sum + bytes, 0);
const report = [
  `# ${path.basename(mapPath, path.extname(mapPath))} OCCT size attribution`,
  "",
  `Map: \`${path.basename(mapPath)}\``,
  "",
  `Total linked OCCT bytes: ${total.toLocaleString("en-US")}`,
  "",
  "| Rank | Toolkit | Linked bytes | KiB | OCCT share |",
  "| ---: | --- | ---: | ---: | ---: |",
  ...rows.map(([toolkit, bytes], index) => (
    `| ${index + 1} | \`${toolkit}\` | ${bytes.toLocaleString("en-US")} | ${(bytes / 1024).toFixed(1)} | ${((bytes / total) * 100).toFixed(2)}% |`
  )),
  "",
].join("\n");

if (outputPath !== undefined) await writeFile(outputPath, report, "utf8");
process.stdout.write(report);
