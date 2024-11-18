import argparse
import os
from data_loader import DataLoader
from visualizer import Visualizer


def main():
    """
    The main entry point for the visualization tool.

    Parses command-line arguments and generates visualizations accordingly.
    """
    parser = argparse.ArgumentParser(
        description="Memory Allocator Performance Visualization Tool"
    )

    # Add arguments
    parser.add_argument(
        '-i', '--input',
        type=str,
        default='performance_data.csv',
        help='Path to the input CSV file containing performance data.'
    )
    parser.add_argument(
        '-o', '--output',
        type=str,
        default=None,
        help='Output directory to save the plots. If not specified, plots are displayed.'
    )
    parser.add_argument(
        '-p', '--plots',
        nargs='+',
        choices=['line', 'bar', 'heatmap', 'all'],
        default=['all'],
        help='Types of plots to generate.'
    )

    args = parser.parse_args()

    # Validates and creates output directory if needed
    if args.output:
        output_dir = os.path.abspath(args.output)
        try:
            if not os.path.isdir(output_dir):
                os.makedirs(output_dir, exist_ok=True)
        except Exception as e:
            print(f"Error creating output directory '{output_dir}': {e}")
            return
    else:
        output_dir = None

    # Loads and preprocesses data
    loader = DataLoader(args.input)
    df = loader.load_data()
    if df is None:
        return
    df = loader.preprocess_data(df)

    # Initializes Visualizer
    viz = Visualizer()

    # Generates plots based on user input
    plots_to_generate = args.plots
    if 'all' in plots_to_generate:
        plots_to_generate = ['line', 'bar', 'heatmap']

    if 'line' in plots_to_generate:
        output_path = os.path.join(output_dir, 'line_plot.png') if output_dir else None
        viz.line_plot_time(df, output_path=output_path)

    if 'bar' in plots_to_generate:
        output_path = os.path.join(output_dir, 'bar_chart.png') if output_dir else None
        viz.bar_chart_block_size(df, output_path=output_path)

    if 'heatmap' in plots_to_generate:
        output_path = os.path.join(output_dir, 'heatmap.png') if output_dir else None
        viz.heatmap_fragmentation(df, output_path=output_path)


if __name__ == '__main__':
    main()