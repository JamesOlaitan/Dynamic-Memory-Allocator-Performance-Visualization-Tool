import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from typing import Optional
import numpy as np
import os
import warnings

# Suppress non-critical warnings (Optional)
warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", category=pd.errors.SettingWithCopyWarning)


class Visualizer:
    """
    A class for generating visualizations from performance data.

    Methods
    -------
    total_memory_usage_over_time(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Plots total memory usage over time.
    allocation_deallocation_rates_over_time(df: pd.DataFrame, interval: str = '1s',
                                            output_path: Optional[str] = None) -> None
        Plots allocation and deallocation rates over time.
    allocation_latency_over_time(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Plots allocation and deallocation latency over time.
    allocation_size_distribution(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Plots the distribution of allocation sizes.
    memory_usage_by_source(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Plots total memory usage by source (module/function/class).
    number_of_allocations_by_source(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Plots the number of allocations per source.
    average_allocation_latency_by_source(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Plots average allocation latency per source.
    allocation_size_vs_time_heatmap(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Creates a heatmap of allocation sizes over time.
    call_stack_trace_frequency(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Plots the frequency of allocations from each call stack.
    throughput_trends(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Plots allocation and deallocation throughput trends over multiple benchmarks.
    """

    def __init__(self):
        """
        Initializes the Visualizer class.
        """
        sns.set_theme(style="whitegrid")

    def set_x_limits(self, plt_obj, timestamps: pd.Series, buffer_ratio: float = 0.05):
        """
        Sets the x-axis limits based on the data's timestamp range with a buffer.

        Parameters
        ----------
        plt_obj : matplotlib.pyplot
            The matplotlib pyplot object.
        timestamps : pd.Series
            The series containing timestamp data.
        buffer_ratio : float, default=0.05
            The ratio of the time span to add as a buffer on both ends.

        Returns
        -------
        None
        """
        min_timestamp = timestamps.min()
        max_timestamp = timestamps.max()
        time_span = max_timestamp - min_timestamp
        buffer = time_span * buffer_ratio

        # Handle cases where time_span is zero or negative
        if time_span.total_seconds() > 0:
            plt_obj.xlim(min_timestamp - buffer, max_timestamp + buffer)
        else:
            # If all timestamps are the same, set arbitrary limits
            plt_obj.xlim(min_timestamp - pd.Timedelta(seconds=1), max_timestamp + pd.Timedelta(seconds=1))

    def total_memory_usage_over_time(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Plots total memory usage over time.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            # Exclude summary logs
            df_sorted = df[df['Operation'] != 'Summary'].sort_values('Timestamp').copy()
            df_sorted['NetMemoryChange'] = df_sorted.apply(
                lambda row: row['BlockSize'] if row['Operation'] == 'Allocation' else -row['BlockSize'], axis=1)
            df_sorted['TotalMemory'] = df_sorted['NetMemoryChange'].cumsum()

            plt.figure(figsize=(12, 6))
            sns.lineplot(data=df_sorted, x='Timestamp', y='TotalMemory')
            plt.title('Total Memory Usage Over Time')
            plt.xlabel('Timestamp')
            plt.ylabel('Total Allocated Memory (bytes)')

            # Set x-axis limits based on data
            self.set_x_limits(plt, df_sorted['Timestamp'])

            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Total memory usage plot saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)  # Wait until the plot window is closed
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the total memory usage plot: {e}")

    def allocation_deallocation_rates_over_time(self, df: pd.DataFrame, interval: str = '1s',
                                                output_path: Optional[str] = None) -> None:
        """
        Plots allocation and deallocation rates over time.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        interval : str, default='1s'
            Time interval for grouping (e.g., '1s' for 1 second).
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            # Exclude summary logs
            df_temp = df[df['Operation'] != 'Summary'].copy()
            df_temp.set_index('Timestamp', inplace=True)
            counts = df_temp.groupby([pd.Grouper(freq=interval.lower()), 'Operation'], observed=False).size().unstack(fill_value=0)

            if counts.empty:
                print("No data available for Allocation/Deallocation Rates Over Time plot.")
                return

            unique_timestamps = counts.index.nunique()
            if unique_timestamps < 2:
                print("Insufficient variation in timestamps for Allocation/Deallocation Rates Over Time plot.")
                return

            plt.figure(figsize=(12, 6))
            counts.plot(kind='line', marker='o')
            plt.title('Allocation and Deallocation Rates Over Time')
            plt.xlabel('Timestamp')
            plt.ylabel('Number of Operations')
            plt.legend(title='Operation')

            # Set x-axis limits based on data
            self.set_x_limits(plt, counts.index)

            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Allocation/deallocation rates plot saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the allocation/deallocation rates plot: {e}")

    def allocation_latency_over_time(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Plots allocation and deallocation latency over time.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            # Exclude summary logs
            df = df[df['Operation'] != 'Summary']
            if df.empty:
                print("No data available for Allocation Latency Over Time plot.")
                return

            plt.figure(figsize=(12, 6))
            sns.lineplot(
                data=df,
                x='Timestamp',
                y='Time',
                hue='Operation',
                style='Operation',
                markers=True,
                dashes=False
            )
            plt.title('Allocation/Deallocation Latency Over Time')
            plt.xlabel('Timestamp')
            plt.ylabel('Latency (seconds)')
            plt.legend(title='Operation')

            # Set x-axis limits based on data
            self.set_x_limits(plt, df['Timestamp'])

            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Allocation latency plot saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the allocation latency plot: {e}")

    def allocation_size_distribution(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Plots the distribution of allocation sizes.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            alloc_df = df[(df['Operation'] == 'Allocation') & (df['Operation'] != 'Summary')]
            if alloc_df.empty:
                print("No allocation data available for Allocation Size Distribution plot.")
                return

            plt.figure(figsize=(10, 6))
            sns.histplot(alloc_df['BlockSize'], bins=30, kde=False)
            plt.title('Allocation Size Distribution')
            plt.xlabel('Block Size (bytes)')
            plt.ylabel('Number of Allocations')
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Allocation size distribution plot saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the allocation size distribution plot: {e}")

    def memory_usage_by_source(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Plots total memory usage by source (module/function/class).

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            alloc_df = df[(df['Operation'] == 'Allocation') & (df['Operation'] != 'Summary')]
            if alloc_df.empty:
                print("No allocation data available for Memory Usage By Source plot.")
                return

            memory_by_source = alloc_df.groupby('Source')['BlockSize'].sum().reset_index()
            if memory_by_source.empty:
                print("No memory usage data available for Memory Usage By Source plot.")
                return

            plt.figure(figsize=(12, 6))
            sns.barplot(
                data=memory_by_source,
                x='Source',
                y='BlockSize'
            )
            plt.title('Total Memory Usage by Source')
            plt.xlabel('Source')
            plt.ylabel('Total Allocated Memory (bytes)')
            plt.xticks(rotation=45, ha='right')
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Memory usage by source plot saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the memory usage by source plot: {e}")

    def number_of_allocations_by_source(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Plots the number of allocations per source.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            alloc_df = df[(df['Operation'] == 'Allocation') & (df['Operation'] != 'Summary')]
            if alloc_df.empty:
                print("No allocation data available for Number of Allocations By Source plot.")
                return

            counts_by_source = alloc_df['Source'].value_counts().reset_index()
            counts_by_source.columns = ['Source', 'AllocationCount']
            if counts_by_source.empty:
                print("No allocation count data available for Number of Allocations By Source plot.")
                return

            plt.figure(figsize=(12, 6))
            sns.barplot(
                data=counts_by_source,
                x='Source',
                y='AllocationCount'
            )
            plt.title('Number of Allocations by Source')
            plt.xlabel('Source')
            plt.ylabel('Number of Allocations')
            plt.xticks(rotation=45, ha='right')
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Number of allocations by source plot saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the number of allocations by source plot: {e}")

    def average_allocation_latency_by_source(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Plots average allocation latency per source.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            alloc_df = df[(df['Operation'] == 'Allocation') & (df['Operation'] != 'Summary')]
            if alloc_df.empty:
                print("No allocation data available for Average Allocation Latency By Source plot.")
                return

            latency_by_source = alloc_df.groupby('Source')['Time'].mean().reset_index()
            if latency_by_source.empty:
                print("No latency data available for Average Allocation Latency By Source plot.")
                return

            plt.figure(figsize=(12, 6))
            sns.barplot(
                data=latency_by_source,
                x='Source',
                y='Time'
            )
            plt.title('Average Allocation Latency by Source')
            plt.xlabel('Source')
            plt.ylabel('Average Latency (seconds)')
            plt.xticks(rotation=45, ha='right')
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Average allocation latency by source plot saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the average allocation latency by source plot: {e}")

    def allocation_size_vs_time_heatmap(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Creates a heatmap of allocation sizes over time.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            alloc_df = df[(df['Operation'] == 'Allocation') & (df['Operation'] != 'Summary')].copy()
            if alloc_df.empty:
                print("No allocation data available for Allocation Size Vs Time Heatmap plot.")
                return

            # Create TimeBin and SizeBin using .loc to avoid SettingWithCopyWarning
            alloc_df.loc[:, 'TimeBin'] = pd.cut(alloc_df['Timestamp'], bins=50)
            alloc_df.loc[:, 'SizeBin'] = pd.cut(alloc_df['BlockSize'], bins=50)

            heatmap_data = alloc_df.pivot_table(
                index='SizeBin',
                columns='TimeBin',
                values='AllocationID',
                aggfunc='count',
                fill_value=0,
                observed=True  # Explicitly set to avoid FutureWarning
            )

            if heatmap_data.empty:
                print("No data available for Allocation Size Vs Time Heatmap plot.")
                return

            # Check if heatmap_data has more than one row and column
            if heatmap_data.shape[0] < 2 or heatmap_data.shape[1] < 2:
                print("Insufficient data variation for Allocation Size Vs Time Heatmap plot.")
                return

            plt.figure(figsize=(12, 8))
            sns.heatmap(
                heatmap_data,
                cmap='YlGnBu',
                cbar_kws={'label': 'Number of Allocations'}
            )
            plt.title('Allocation Size vs. Time Heatmap')
            plt.xlabel('Time')
            plt.ylabel('Allocation Size')
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Allocation size vs. time heatmap saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the allocation size vs. time heatmap: {e}")

    def call_stack_trace_frequency(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Plots the frequency of allocations from each call stack.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            alloc_df = df[(df['Operation'] == 'Allocation') & (df['Operation'] != 'Summary')]
            if alloc_df.empty:
                print("No allocation data available for Call Stack Trace Frequency plot.")
                return

            callstack_counts = alloc_df['CallStack'].value_counts().reset_index()
            callstack_counts.columns = ['CallStack', 'AllocationCount']
            if callstack_counts.empty:
                print("No call stack trace data available for Call Stack Trace Frequency plot.")
                return

            plt.figure(figsize=(12, 6))
            sns.barplot(
                data=callstack_counts,
                x='CallStack',
                y='AllocationCount'
            )
            plt.title('Allocation Frequency by Call Stack Trace')
            plt.xlabel('Call Stack Trace')
            plt.ylabel('Number of Allocations')
            plt.xticks(rotation=45, ha='right')
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Call stack trace frequency plot saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the call stack trace frequency plot: {e}")

    def throughput_trends(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Plots allocation and deallocation throughput trends over multiple benchmarks.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data, including summary logs.
        output_path : Optional[str], default=None
            The file path to save the throughput trends plot image. If None, the plot is displayed.

        Returns
        -------
        None
        """
        try:
            # Filter summary logs
            summary_df = df[df['Operation'] == 'Summary'].copy()
            if summary_df.empty:
                print("No summary data available for Throughput Trends plot.")
                return

            # Extract throughput and fragmentation from summary logs
            # Assuming 'Time' = allocThroughput, 'Fragmentation' = deallocThroughput,
            # 'Source' = fragmentation percentage, 'CallStack' = summary description

            summary_df['AllocThroughput'] = summary_df['Time']
            summary_df['DeallocThroughput'] = summary_df['Fragmentation']
            summary_df['Fragmentation'] = summary_df['Source']
            summary_df['BenchmarkDescription'] = summary_df['CallStack']

            # Convert 'Timestamp' to datetime if not already
            if not np.issubdtype(summary_df['Timestamp'].dtype, np.datetime64):
                summary_df['Timestamp'] = pd.to_datetime(summary_df['Timestamp'])

            # Sort by Timestamp
            summary_df.sort_values('Timestamp', inplace=True)

            # Plot Allocation Throughput Over Time
            plt.figure(figsize=(12, 6))
            sns.lineplot(data=summary_df, x='Timestamp', y='AllocThroughput', marker='o', label='Allocation Throughput')
            sns.lineplot(data=summary_df, x='Timestamp', y='DeallocThroughput', marker='o', label='Deallocation Throughput')
            plt.title('Throughput Trends Over Time')
            plt.xlabel('Benchmark Timestamp')
            plt.ylabel('Throughput (operations per second)')
            plt.legend()
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Throughput trends plot saved to {os.path.abspath(output_path)}")
                plt.close()
            else:
                plt.show(block=True)
                plt.close()

        except Exception as e:
            print(f"An error occurred while generating the throughput trends plot: {e}")