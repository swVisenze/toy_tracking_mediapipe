"""
Script for end-to-end evaluation
"""
import typer
from path import Path


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
    )
):
    ...


if __name__ == "__main__":
    typer.run(main)
