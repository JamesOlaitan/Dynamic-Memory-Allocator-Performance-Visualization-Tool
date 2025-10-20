import argparse
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import List

PROJECT_ROOT = Path(__file__).resolve().parents[2]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from scripts.data_loader import DataLoader
from scripts.visualizer import Visualizer

def main():
    """
    The main entry point for the visualization tool.

    Parses command-line arguments and generates visualizations accordingly.
    """
    parser = argparse.ArgumentParser(
        description="Memory Allocator Performance Visualization Tool"
    )

    # Modify the input argument to accept multiple CSV files and make it optional
    parser.add_argument(
        '-i', '--input',
        type=str,
        nargs='+',
        default=[],
        help='Path(s) to the input CSV file(s) containing performance data.'
    )
    parser.add_argument(
        '-o', '--output',
        type=str,
        default='reports/plots', 
        help='Output directory to save the plots. Defaults to "reports/plots".'
    )
    parser.add_argument(
        '--timestamp',
        action='store_true',
        help='Add timestamp subdirectory to output path.'
    )
    parser.add_argument(
        '-p', '--plots',
        nargs='+',
        choices=[
            'memory_usage_over_time',
            'allocation_deallocation_rates',
            'allocation_latency_over_time',
            'allocation_latency_percentiles',
            'allocation_size_distribution',
            'memory_usage_by_source',
            'number_of_allocations_by_source',
            'average_allocation_latency_by_source',
            'allocation_size_vs_time_heatmap',
            'call_stack_trace_frequency',
            'throughput_trends',
            'all'
        ],
        default=['all'],
        help='Types of plots to generate. Choose from the list or select "all" for all plots.'
    )

    args = parser.parse_args()

    # Determine which CSV files to use
    if args.input:
        csv_files = args.input
        print(f"User provided {len(csv_files)} CSV file(s) for processing.")
    else:
        # Look for CSV files in reports directory
        reports_dir = "reports"
        if os.path.exists(reports_dir):
            csv_files = [os.path.join(reports_dir, f) for f in os.listdir(reports_dir) 
                        if f.endswith('.csv')]
            if csv_files:
                print(f"No input files provided. Found {len(csv_files)} CSV file(s) in reports/ directory.")
            else:
                print("No input CSV files provided and no CSV files found in reports/ directory. Exiting.")
                return
        else:
            print("No input CSV files provided and reports/ directory not found. Exiting.")
            return

    # Validate and create output directory if needed
    output_dir = os.path.abspath(args.output)
    
    # Add timestamp subdirectory if requested
    if args.timestamp:
        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        output_dir = os.path.join(output_dir, timestamp)
    try:
        if not os.path.isdir(output_dir):
            os.makedirs(output_dir, exist_ok=True)
            print(f"Created output directory at: {output_dir}")
        else:
            print(f"Using existing output directory at: {output_dir}")
    except Exception as e:
        print(f"Error creating output directory '{output_dir}': {e}")
        return

    # Initialize Visualizer
    viz = Visualizer()

    # Determine plots to generate
    plots_to_generate = args.plots
    if 'all' in plots_to_generate:
        plots_to_generate = [
            'memory_usage_over_time',
            'allocation_deallocation_rates',
            'allocation_latency_over_time',
            'allocation_latency_percentiles',
            'allocation_size_distribution',
            'memory_usage_by_source',
            'number_of_allocations_by_source',
            'average_allocation_latency_by_source',
            'allocation_size_vs_time_heatmap',
            'call_stack_trace_frequency',
            'throughput_trends'
        ]

    # Mapping of plot types to Visualizer methods and output filenames
    plot_methods = {
        'memory_usage_over_time': {
            'method': viz.total_memory_usage_over_time,
            'filename': 'total_memory_usage_over_time.png'
        },
        'allocation_deallocation_rates': {
            'method': viz.allocation_deallocation_rates_over_time,
            'filename': 'allocation_deallocation_rates_over_time.png'
        },
        'allocation_latency_over_time': {
            'method': viz.allocation_latency_over_time,
            'filename': 'allocation_latency_over_time.png'
        },
        'allocation_latency_percentiles': {
            'method': viz.allocation_latency_percentiles,
            'filename': 'allocation_latency_percentiles.png'
        },
        'allocation_size_distribution': {
            'method': viz.allocation_size_distribution,
            'filename': 'allocation_size_distribution.png'
        },
        'memory_usage_by_source': {
            'method': viz.memory_usage_by_source,
            'filename': 'memory_usage_by_source.png'
        },
        'number_of_allocations_by_source': {
            'method': viz.number_of_allocations_by_source,
            'filename': 'number_of_allocations_by_source.png'
        },
        'average_allocation_latency_by_source': {
            'method': viz.average_allocation_latency_by_source,
            'filename': 'average_allocation_latency_by_source.png'
        },
        'allocation_size_vs_time_heatmap': {
            'method': viz.allocation_size_vs_time_heatmap,
            'filename': 'allocation_size_vs_time_heatmap.png'
        },
        'call_stack_trace_frequency': {
            'method': viz.call_stack_trace_frequency,
            'filename': 'call_stack_trace_frequency.png'
        },
        'throughput_trends': {
            'method': viz.throughput_trends,
            'filename': 'throughput_trends.png'
        }
    }

    # Iterate over each CSV file
    for csv_file in csv_files:
        if not os.path.exists(csv_file):
            print(f"CSV file '{csv_file}' does not exist. Skipping.")
            continue

        print(f"\nProcessing CSV file: {csv_file}")

        # Load and preprocess data
        loader = DataLoader(csv_file)
        df = loader.load_data()
        if df is None or df.empty:
            print(f"No data loaded from '{csv_file}'. Skipping.")
            continue
        df = loader.preprocess_data(df)
        if df is None or df.empty:
            print(f"Data preprocessing resulted in an empty DataFrame for '{csv_file}'. Skipping.")
            continue

        # Extract base name without extension for plot naming
        base_name = os.path.splitext(os.path.basename(csv_file))[0]

        # Generate plots based on user input
        for plot_type in plots_to_generate:
            if plot_type in plot_methods:
                method = plot_methods[plot_type]['method']
                filename = plot_methods[plot_type]['filename']
                # Prefix the plot filename with the CSV base name to avoid conflicts
                prefixed_filename = f"{base_name}_{filename}"
                output_path = os.path.join(output_dir, prefixed_filename)
                readable_plot_name = plot_type.replace('_', ' ').title()
                print(f"Generating plot: {readable_plot_name} -> {prefixed_filename}")
                method(df, output_path=output_path)
            else:
                print(f"Plot type '{plot_type}' is not recognized and will be skipped.")

    print("\nAll requested plots have been generated.")

if __name__ == '__main__':
    main()
