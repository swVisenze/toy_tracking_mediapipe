"""
Script for end-to-end evaluation
"""
import typer
import subprocess
from path import Path
from joblib import Parallel, delayed


VIDEO_EXT = "mp4"
EXE = "bazel-bin/mediapipe/examples/desktop/object_tracking/toy_tracking_cpu"
GRAPH_CONFIG_FILE = "mediapipe/graphs/tracking/toy_detection_tracking_desktop_live.pbtxt"


def exec_inference(video_path, workdir):
    """ Run e2e inference and output both video and csv files

    Parameters:
        * video_path: (Path) input video path
        * workdir: (Path) output working dir
    Return: None
    """
    output_path = workdir / video_path.stem
    subprocess.run([
        EXE,
        f"--calculator_graph_config_file={GRAPH_CONFIG_FILE}",
        f"--input_video_path={video_path}",
        f"--output_video_path={output_path}"
    ])


def main(
    input_folder: Path = typer.Argument(
        ...,
        exists=True,
        dir_okay=True,
        readable=True,
        help="Input folder containing all test videos",
        path_type=Path
    ),
    workdir: Path = typer.Argument(
        ...,
        exists=False,
        dir_okay=True,
        writable=True,
        help="Output folder",
        path_type=Path
    ),
    description: str = typer.Argument(
        ...,
        help="Discription of the experiment"
    )
):
    workdir.makedirs_p()
    with open(workdir / "report.txt", "w") as ftxt:
        ftxt.write(description)

    subprocess.run([
        "bazel",
        "build",
        "-c",
        "opt",
        "--define",
        "MEDIAPIPE_DISABLE_GPU=1",
        "mediapipe/examples/desktop/object_tracking:toy_tracking_cpu"
    ], check=True)
    Parallel(n_jobs=1, prefer="processes")(
        delayed(exec_inference)(video_path, workdir)
        for video_path in input_folder.walkfiles(f"*.{VIDEO_EXT}")
    )
    print("== Done ==")


if __name__ == "__main__":
    typer.run(main)
