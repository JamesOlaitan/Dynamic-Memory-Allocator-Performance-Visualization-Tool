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
        default='plots', 
        help='Output directory to save the plots. If not specified, plots are displayed.'
    )
    parser.add_argument(
        '-p', '--plots',
        nargs='+',
        choices=[
            'memory_usage_over_time',
            'allocation_deallocation_rates',
            'allocation_latency_over_time',
            'allocation_size_distribution',
            'memory_usage_by_source',
            'number_of_allocations_by_source',
            'average_allocation_latency_by_source',
            'allocation_size_vs_time_heatmap',
            'call_stack_trace_frequency',
            'all'
        ],
        default=['all'],
        help='Types of plots to generate. Choose from the list or select "all" for all plots.'
    )

    args = parser.parse_args()

    # Validates and creates output directory if needed
    output_dir = os.path.abspath(args.output)
    try:
        if not os.path.isdir(output_dir):
            os.makedirs(output_dir, exist_ok=True)
            print(f"Created output directory at: {output_dir}")
        else:
            print(f"Using existing output directory at: {output_dir}")
    except Exception as e:
        print(f"Error creating output directory '{output_dir}': {e}")
        return

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
        plots_to_generate = [
            'memory_usage_over_time',
            'allocation_deallocation_rates',
            'allocation_latency_over_time',
            'allocation_size_distribution',
            'memory_usage_by_source',
            'number_of_allocations_by_source',
            'average_allocation_latency_by_source',
            'allocation_size_vs_time_heatmap',
            'call_stack_trace_frequency'
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
        }
    }

    for plot_type in plots_to_generate:
        if plot_type in plot_methods:
            method = plot_methods[plot_type]['method']
            filename = plot_methods[plot_type]['filename']
            output_path = os.path.join(output_dir, filename)
            readable_plot_name = plot_type.replace('_', ' ').title()
            print(f"Generating plot: {readable_plot_name}")
            method(df, output_path=output_path)
        else:
            print(f"Plot type '{plot_type}' is not recognized and will be skipped.")

    print("All requested plots have been generated.")

if __name__ == '__main__':
    main()